/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXTERN_WORKER_INL_H_
#error "extern-worker-inl.h should only be included by extern-worker.h"
#endif

#include "hphp/util/assertions.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <folly/Conv.h>
#include <folly/container/Foreach.h>
#include <folly/gen/Base.h>

#include <filesystem>
#include <tuple>
#include <type_traits>

namespace HPHP::extern_worker {

//////////////////////////////////////////////////////////////////////

template <typename C>
Job<C>::Job() : detail::JobBase{C::name()} {}

// These functions in Job are responsible for taking paths
// representing the data, turning them into whatever types that "C"
// expects, then invoke the function with those types.

template <typename C>
void Job<C>::init(detail::ISource& source) const {
  using namespace detail;
  using Args = typename Params<decltype(C::init)>::type;

  // For each expected input, load it, and deserialize it into
  // the appropriate type. Return the types as a tuple, which can then
  // std::apply to C::init, passing the inputs.
  size_t DEBUG_ONLY nextIdx = 0;
  std::apply(
    C::init,
    typesToValues<Args>(
      [&] (size_t idx, auto tag) {
        assertx(idx == nextIdx++);
        return deserialize<typename decltype(tag)::Type>(source);
      }
    )
  );
  source.initDone();

  using Ret = typename Return<decltype(C::init)>::type;
  static_assert(std::is_void_v<Ret>, "init() must return void");
}

template <typename C>
void Job<C>::fini(detail::ISink& sink) const {
  using namespace detail;

  sink.startFini();
  using Ret = typename Return<decltype(C::fini)>::type;
  if constexpr (std::is_void_v<Ret>) {
    C::fini();
  } else {
    auto const v = C::fini();
    time("writing fini outputs", [&] { return serialize(v, sink); });
  }
}

template <typename C>
void Job<C>::run(detail::ISource& source, detail::ISink& sink) const {
  using namespace detail;

  // For each expected input, load it, and deserialize it into the
  // appropriate type, turning all of the types into a tuple.
  using Args = typename Params<decltype(C::run)>::type;
  size_t DEBUG_ONLY nextIdx = 0;
  auto inputs = time(
    "loading inputs",
    [&] {
      return typesToValues<Args>(
        [&] (size_t idx, auto tag) {
          assertx(idx == nextIdx++);
          return deserialize<typename decltype(tag)::Type>(source);
        }
      );
    }
  );
  source.nextInput();

  // Apply the tuple to C::run, passing the types as parameters.
  auto outputs = time(
    "actual run",
    [&] { return std::apply(C::run, std::move(inputs)); }
  );

  using Ret = typename Return<decltype(C::run)>::type;
  static_assert(!std::is_void_v<Ret>, "run() must return something");

  // Serialize the outputs
  time("writing outputs", [&] { return serialize(outputs, sink); });
  sink.nextOutput();
}

//////////////////////////////////////////////////////////////////////

namespace detail {

//////////////////////////////////////////////////////////////////////

// Turn a blob into a specific (non-marker) type
template <typename T>
T JobBase::deserializeBlob(std::string blob) {
  using namespace detail;
  static_assert(!IsMarker<T>::value, "Special markers cannot be nested");
  if constexpr (std::is_same<T, std::string>::value) {
    // A std::string is always stored as itself (this lets us store
    // files directly as their contents without having to encode
    // them).
    return blob;
  } else {
    // For most types, the data is encoded using BlobEncoder, so undo
    // that.
    BlobDecoder decoder{blob.data(), blob.size()};
    return decoder.makeWhole<T>();
  }
}

// Deserialize the given input source into the type T and return
// it. The type might include markers, which might trigger
// sub-deserializations.
template <typename T>
T JobBase::deserialize(ISource& source) {
  using namespace detail;
  static_assert(!IsMulti<T>::value, "Multi can only be used as return type");

  if constexpr (IsVariadic<T>::value) {
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    auto const blobs = source.variadic();
    T out;
    out.vals.reserve(blobs.size());
    for (auto const& blob : blobs) {
      out.vals.emplace_back(deserializeBlob<typename T::Type>(blob));
    }
    return out;
  } else if constexpr (IsOpt<T>::value) {
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    // Opt<T> is like T, except the data may not exist (so is nullopt
    // otherwise).
    T out;
    if (auto const blob = source.optBlob()) {
      out.val.emplace(deserializeBlob<typename T::Type>(*blob));
    }
    return out;
  } else {
    return deserializeBlob<T>(source.blob());
  }
}

// Serialize the given (non-marker) value into a blob
template <typename T>
std::string JobBase::serializeBlob(const T& v) {
  using namespace detail;
  static_assert(!IsMarker<T>::value,
                "Special markers cannot be nested");
  if constexpr (std::is_same<T, std::string>::value) {
    // std::string always encodes to itself
    return v;
  } else {
    BlobEncoder encoder;
    encoder(v);
    return std::string{(const char*)encoder.data(), encoder.size()};
  }
}

// Serialize the given value into a blob and write it out to the given
// output sink. The value's type might be a marker, which can trigger
// sub-serializations.
template <typename T>
void JobBase::serialize(const T& v, ISink& sink) {
  using namespace detail;
  if constexpr (IsVariadic<T>::value) {
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    using namespace folly::gen;
    auto const blobs = from(v.vals)
      | map([&] (const typename T::Type& t) { return serializeBlob(t); })
      | as<std::vector>();
    sink.variadic(blobs);
  } else if constexpr (IsOpt<T>::value) {
    // Opt<T> is like T, except nothing is written if the value isn't
    // present.
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    sink.optBlob(
      v.val.has_value() ? serializeBlob(*v.val) : Optional<std::string>{}
    );
  } else if constexpr (IsMulti<T>::value) {
    // Treat Multi as equivalent to std::tuple (IE, write each element
    // separately).
    size_t DEBUG_ONLY nextTupleIdx = 0;
    for_each(
      v.vals,
      [&] (auto const& elem, size_t tupleIdx) {
        static_assert(
          !IsMulti<
            std::remove_cv_t<std::remove_reference_t<decltype(elem)>>
          >::value,
         "Multi cannot be nested"
        );
        assertx(tupleIdx == nextTupleIdx++);
        serialize(elem, sink);
      }
    );
  } else {
    sink.blob(serializeBlob(v));
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

inline
RefId::RefId(const std::array<uint8_t, RefId::kDigestLen>& digest, size_t size)
  : m_id{(const char*)digest.data(), digest.size()}
  , m_size{size}
  , m_extra{RefId::kDigestSentinel}
{}

inline RefId::RefId(std::string id, size_t size, size_t extra)
  : m_id{std::move(id)}, m_size{size}, m_extra{extra}
{}

inline bool RefId::operator==(const RefId& o) const {
  return
    std::tie(m_id, m_extra, m_size) ==
    std::tie(o.m_id, o.m_extra, o.m_size);
}

inline bool RefId::operator!=(const RefId& o) const {
  return !(*this == o);
}

inline bool RefId::operator<(const RefId& o) const {
  return
    std::tie(m_id, m_extra, m_size) <
    std::tie(o.m_id, o.m_extra, o.m_size);
}

inline bool RefId::operator<=(const RefId& o) const {
  return
    std::tie(m_id, m_extra, m_size) <=
    std::tie(o.m_id, o.m_extra, o.m_size);
}

inline size_t RefId::hash() const {
  return folly::hash::hash_combine(m_id, m_size, m_extra);
}

inline std::string RefId::toString() const {
  // Don't print out the extra field if it's zero, to avoid clutter
  // for implementations which don't use it. The id might contain
  // binary data, so escape it before printing.
  switch (m_extra) {
  case kDigestSentinel:
    return folly::sformat("{}:{}", folly::hexlify(m_id), m_size);
  case 0:
    return folly::sformat("{}:{}", folly::humanify(m_id), m_size);
  default:
    return folly::sformat("{}:{}:{}", folly::humanify(m_id), m_extra, m_size);
  }
}

//////////////////////////////////////////////////////////////////////

template <typename T>
inline Ref<T>::Ref(RefId id)
  : m_id{std::move(id)}
{
  static_assert(!detail::IsMarker<T>::value,
                "Special markers cannot be used in a Ref");
}

//////////////////////////////////////////////////////////////////////

inline RequestId::RequestId(const char* type)
  : m_id{s_next++}
  , m_type{type}
{
  ++s_active;
  FTRACE(2, "{} begin\n", tracePrefix());
  m_timer.emplace();
}

inline RequestId::~RequestId() {
  m_timer->stopWithMessage(
    [this] { return folly::sformat("{} done", tracePrefix()); }
  );
  --s_active;
}

inline std::string RequestId::tracePrefix() const {
  return folly::sformat(
    "[{} req #{} {} ({} active)]",
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch()
    ).count(),
    m_id,
    m_type,
    s_active.load()
  );
}

inline std::string RequestId::toString() const {
  return folly::to<std::string>(m_id);
}

inline RequestId::Clock::duration RequestId::elapsed() const {
  return m_timer->elapsed();
}

//////////////////////////////////////////////////////////////////////

inline const std::string& Client::implName() const {
  return m_impl->name();
}

inline std::string Client::session() const {
  return m_impl->session();
}

inline bool Client::usingSubprocess() const {
  return m_impl->isSubprocess();
}

inline bool Client::supportsOptimistic() const {
  return m_impl->supportsOptimistic();
}

// Run the given callable, retrying if Throttle is thrown, until the
// configured retry limit is reached.
template <typename T, typename F>
folly::coro::Task<T> Client::tryWithThrottling(const F& f) {
  co_return co_await
    Impl::tryWithThrottling<T>(
      m_options.m_throttleRetries,
      m_options.m_throttleBaseWait,
      m_stats->throttles,
      f
    );
}

template <typename T>
folly::coro::Task<T> Client::load(Ref<T> r) {
  RequestId requestId{"load blob"};

  ++m_stats->loadCalls;
  SCOPE_EXIT {
    m_stats->loadLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  // Get the implementation, and forward the request to it.
  auto& impl = *m_impl;
  auto result = co_await tryWithThrottling<BlobVec>(
    [&] { return impl.load(requestId, IdVec{r.m_id}); }
  );
  assertx(result.size() == 1);
  FTRACE(4, "{} blob is {} bytes\n",
         requestId.tracePrefix(), result[0].size());
  ++m_stats->downloads;
  m_stats->bytesDownloaded += result[0].size();
  co_return unblobify<T>(std::move(result[0]));
}

template <typename T, typename... Ts>
folly::coro::Task<std::tuple<T, Ts...>> Client::load(Ref<T> r, Ref<Ts>... rs) {
  using namespace detail;

  RequestId requestId{"load blobs"};
  auto const numBlobs = sizeof...(Ts) + 1;
  FTRACE(2, "{} {} blobs requested\n", requestId.tracePrefix(), numBlobs);
  ++m_stats->loadCalls;
  SCOPE_EXIT {
    m_stats->loadLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  // Retrieve the ids from the Refs
  IdVec ids;
  ids.reserve(numBlobs);
  for_each(std::make_tuple(std::move(r), std::move(rs)...),
    [&] (auto&& r, size_t idx) {
      ids.emplace_back(std::move(r.m_id));
    }
  );

  // Do the loads
  auto blobs = co_await tryWithThrottling<BlobVec>(
    [&] { return m_impl->load(requestId, ids); }
  );

  assertx(blobs.size() == numBlobs);
  using OutTuple = std::tuple<T, Ts...>;

  // Now restore the results back into the output tuple.
  auto ret = typesToValues<OutTuple>([&] (size_t idx, auto tag) {
    assertx(idx < blobs.size());
    auto& blob = blobs[idx];

    FTRACE(4, "{} blob #{} is {} bytes\n",
           requestId.tracePrefix(), idx, blob.size());
    m_stats->bytesDownloaded += blob.size();
    // Turn it into the value.
    return unblobify<typename decltype(tag)::Type>(std::move(blob));
  });

  m_stats->downloads += numBlobs;
  co_return ret;
}

template <typename T>
folly::coro::Task<std::vector<T>> Client::load(std::vector<Ref<T>> rs) {
  using namespace folly::gen;

  RequestId requestId{"load blobs"};
  FTRACE(2, "{} {} blobs requested\n", requestId.tracePrefix(), rs.size());
  ++m_stats->loadCalls;
  SCOPE_EXIT {
    m_stats->loadLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  // Retrieve the RefId from the refs
  IdVec ids;
  for (auto& r : rs) {
    ids.emplace_back(std::move(r.m_id));
  }

  auto const DEBUG_ONLY numBlobs = ids.size();
  assertx(numBlobs == rs.size());

  // Do the loads
  auto blobs = co_await tryWithThrottling<BlobVec>(
    [&] { return m_impl->load(requestId, ids); }
  );
  assertx(blobs.size() == numBlobs);

  std::vector<T> out;
  out.reserve(numBlobs);

  // Return blobs in the same order their Refs were passed to this function.
  for (size_t i = 0; i < numBlobs; ++i) {
    auto& blob = blobs[i];
    FTRACE(4, "{} blob #{} is {} bytes\n",
           requestId.tracePrefix(), out.size(), blob.size());
    m_stats->bytesDownloaded += blob.size();
    out.emplace_back(unblobify<T>(std::move(blob)));
  }
  assertx(out.size() == numBlobs);

  m_stats->downloads += numBlobs;
  co_return out;
}

template <typename T, typename... Ts>
folly::coro::Task<std::vector<std::tuple<T, Ts...>>>
Client::load(std::vector<std::tuple<Ref<T>, Ref<Ts>...>> rs) {
  using namespace detail;
  using namespace folly::gen;

  using OutTuple = std::tuple<T, Ts...>;
  auto constexpr tupleSize = std::tuple_size<OutTuple>{};
  auto numTuples = rs.size();

  RequestId requestId{"load blobs"};
  FTRACE(2, "{} {} blobs requested\n",
         requestId.tracePrefix(), numTuples * tupleSize);
  ++m_stats->loadCalls;
  SCOPE_EXIT {
    m_stats->loadLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  // This is a hybrid of the variadic case and the vector case:
  // Retrieve the RefId from the refs. "Flatten" out the refs into flat lists,
  // disregarding the tuple structure (which will be rebuilt later).
  IdVec ids;
  for (size_t i = 0; i < numTuples; ++i) {
    for_each(std::move(rs[i]),
      [&] (auto&& r, size_t tupleIdx) {
        ids.emplace_back(std::move(r.m_id));
      }
    );
  }

  auto const numBlobs = ids.size();
  assertx(numBlobs == numTuples * tupleSize);

  // Do the loads
  auto blobs = co_await tryWithThrottling<BlobVec>(
    [&] { return m_impl->load(requestId, ids); }
  );
  assertx(blobs.size() == numBlobs);

  // Now combine blobs back together, in the same order as their associated
  // Refs were given (also restoring the tuple structure of the inputs).
  std::vector<OutTuple> out;
  out.reserve(numTuples);

  for (size_t i = 0; i < numTuples; ++i) {
    out.emplace_back(
      typesToValues<OutTuple>(
        [&] (size_t tupleIdx, auto tag) {
          // This is similar to logic for load(Ref<T>, Ref<T2>, ...)
          auto const idx = i * tupleSize + tupleIdx;
          assertx(idx < blobs.size());
          auto& blob = blobs[idx];

          FTRACE(4, "{} blob #{} is {} bytes\n",
                 requestId.tracePrefix(), idx, blob.size());
          m_stats->bytesDownloaded += blob.size();
          return unblobify<typename decltype(tag)::Type>(std::move(blob));
        }
      )
    );
  }
  assertx(out.size() == numTuples);
  m_stats->downloads += numBlobs;
  co_return out;
}

// All of the store functions are similar: Serialize the inputs into
// blobs, call the store function on the implementation, then wrap the
// output RefIds into Refs with the appropriate types.

template <typename T>
folly::coro::Task<Ref<T>> Client::storeImpl(bool optimistic, T t) {
  RequestId requestId{"store blob"};

  ++m_stats->blobs;
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto ids = co_await tryWithThrottling<IdVec>([&] {
    auto blob = blobify(t);
    FTRACE(2, "{} blob is {} bytes\n", requestId.tracePrefix(), blob.size());
    return m_impl->store(
      requestId,
      {},
      BlobVec{std::move(blob)},
      optimistic
    );
  });
  assertx(ids.size() == 1);

  Ref<T> ref{std::move(ids[0])};
  co_return ref;
}

template <typename T, typename... Ts>
folly::coro::Task<std::tuple<Ref<T>, Ref<Ts>...>>
Client::storeImpl(bool optimistic,
                  T t,
                  Ts... ts) {
  using namespace detail;
  RequestId requestId{"store blobs"};

  FTRACE(2, "{} storing {} blobs\n",
         requestId.tracePrefix(), sizeof...(Ts) + 1);

  m_stats->blobs += (sizeof...(Ts) + 1);
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto ids = co_await tryWithThrottling<IdVec>([&] {
    BlobVec blobs{{ blobify(t), blobify(ts)... }};
    ONTRACE(4, [&] {
      for (auto const& b : blobs) {
        FTRACE(4, "{} storing {} byte blob\n",
               requestId.tracePrefix(), b.size());
      }
    }());
    return m_impl->store(
      requestId,
      {},
      std::move(blobs),
      optimistic
    );
  });
  assertx(ids.size() == sizeof...(Ts) + 1);

  using OutTuple = std::tuple<T, Ts...>;
  auto ret = typesToValues<OutTuple>(
    [&] (size_t idx, auto tag) {
      assertx(idx < ids.size());
      return Ref<typename decltype(tag)::Type>{std::move(ids[idx])};
    }
  );
  co_return ret;
}

template <typename T>
folly::coro::Task<Ref<T>> Client::store(T t) {
  return storeImpl(false, std::move(t));
}

template <typename T, typename... Ts>
folly::coro::Task<std::tuple<Ref<T>, Ref<Ts>...>> Client::store(T t,
                                                                Ts... ts) {
  return storeImpl(false, std::move(t), std::move(ts)...);
}

template <typename T>
folly::coro::Task<Ref<T>> Client::storeOptimistically(T t) {
  return storeImpl(true, std::move(t));
}

template <typename T, typename... Ts>
folly::coro::Task<std::tuple<Ref<T>, Ref<Ts>...>>
Client::storeOptimistically(T t,
                            Ts... ts) {
  return storeImpl(true, std::move(t), std::move(ts)...);
}

template <typename T>
folly::coro::Task<std::vector<Ref<T>>> Client::storeMulti(std::vector<T> ts,
                                                          bool optimistic) {
  using namespace folly::gen;

  RequestId requestId{"store blobs"};

  FTRACE(
    2, "{} storing {} blobs{}\n",
    requestId.tracePrefix(),
    ts.size(),
    optimistic ? " (optimistically)" : ""
  );

  m_stats->blobs += ts.size();
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto ids = co_await tryWithThrottling<IdVec>([&] {
    auto blobs = from(ts)
      | mapped([&] (const T& t) {
          auto blob = blobify(t);
          FTRACE(4, "{} storing {} byte blob\n",
                 requestId.tracePrefix(), blob.size());
          return blob;
        })
      | as<std::vector>();
    assertx(blobs.size() == ts.size());
    return m_impl->store(
      requestId,
      {},
      std::move(blobs),
      optimistic
    );
  });
  assertx(ids.size() == ts.size());

  auto out = from(ids)
    | move
    | mapped([&] (RefId&& id) { return Ref<T>{std::move(id)}; })
    | as<std::vector>();
  co_return out;
}

template <typename T, typename... Ts>
folly::coro::Task<std::vector<std::tuple<Ref<T>, Ref<Ts>...>>>
Client::storeMultiTuple(std::vector<std::tuple<T, Ts...>> ts,
                        bool optimistic) {
  using namespace folly::gen;
  using namespace detail;

  RequestId requestId{"store blobs"};

  using OutTuple = std::tuple<T, Ts...>;
  auto constexpr tupleSize = std::tuple_size<OutTuple>{};

  FTRACE(
    2, "{} storing {} blobs{}\n",
    requestId.tracePrefix(),
    ts.size() * tupleSize,
    optimistic ? " (optimistically)" : ""
  );

  m_stats->blobs += (ts.size() * tupleSize);
  ++m_stats->storeCalls;
  SCOPE_EXIT {
    m_stats->storeLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  auto ids = co_await tryWithThrottling<IdVec>([&] {
    // Map each tuple to a vector of RefIds, then concat all of the
    // vectors together to get one flat list.
    auto to_store = from(ts)
      | mapped([&] (auto const& tuple) {
          BlobVec blobs;
          blobs.reserve(tupleSize);
          for_each(
            tuple,
            [&] (auto const& t) {
              auto blob = blobify(t);
              FTRACE(4, "{} storing {} byte blob\n",
                     requestId.tracePrefix(), blob.size());
              blobs.emplace_back(std::move(blob));
            }
          );
          return fromCopy(std::move(blobs));
        })
      | concat
      | as<std::vector>();
    assertx(to_store.size() == ts.size() * tupleSize);
    return m_impl->store(
      requestId,
      {},
      std::move(to_store),
      optimistic
    );
  });
  assertx(ids.size() == ts.size() * tupleSize);

  // Do the opposite to the output. Batch the output into the
  // tupleSize, then turn each one into a tuple of Refs.
  auto out = from(ids)
    | move
    | batch(tupleSize)
    | move
    | mapped([&] (std::vector<RefId>&& ids) {
        assertx(ids.size() == tupleSize);
        return typesToValues<OutTuple>(
          [&] (size_t idx, auto tag) {
            assertx(idx < ids.size());
            return Ref<typename decltype(tag)::Type>{std::move(ids[idx])};
          }
        );
      })
    | as<std::vector>();
  co_return out;
}

template <typename C> folly::coro::Task<typename Job<C>::ExecT>
Client::exec(const Job<C>& job,
             typename Job<C>::ConfigT config,
             std::vector<typename Job<C>::InputsT> inputs,
             Client::ExecMetadata metadata) {
  using namespace folly::gen;
  using namespace detail;

  RequestId requestId{"exec"};
  FTRACE(2, "{} executing \"{}\" ({} runs)\n",
         requestId.tracePrefix(), job.name(),
         inputs.size());

  m_stats->execWorkItems += inputs.size();
  ++m_stats->execCalls;
  SCOPE_EXIT {
    m_stats->execLatencyUsec += std::chrono::duration_cast<
      std::chrono::microseconds
    >(requestId.elapsed()).count();
  };

  // RefVals are wrappers around a RefId. They provide the RefId,
  // along with the knowledge of whether that RefId corresponds to a
  // Variadic, Opt, or a normal type (the implementation does not know
  // anything about the types of the job its executing). They're just
  // a variant of RefId, Optional<RefId>, or std::vector<RefId>,
  // mirroring the special types.

  auto const toRefVal = [] (auto const& t) -> RefVal {
    using Type = std::remove_cv_t<std::remove_reference_t<decltype(t)>>;
    if constexpr (IsVector<Type>::value) {
      IdVec v;
      v.reserve(t.size());
      for (auto const& e : t) v.emplace_back(e.m_id);
      return v;
    } else if constexpr (IsOptional<Type>::value) {
      if (!t.has_value()) return std::nullopt;
      return make_optional(t->m_id);
    } else {
      static_assert(IsRef<Type>::value);
      return t.m_id;
    }
  };

  auto const toRefVals = [&] (auto const& tuple) {
    RefValVec v;
    v.reserve(std::tuple_size<std::remove_reference_t<decltype(tuple)>>{});
    for_each(
      std::move(tuple),
      [&] (auto const& t) { v.emplace_back(toRefVal(t)); }
    );
    return v;
  };

  // OutputType is similar to RefVals, but for outputs. Since there's
  // no RefId (yet) for the outputs, we can use a simple enum to
  // describe the output format.
  auto const makeOutputTypes = [&] (auto tag) -> folly::Range<const OutputType*> {
    using T = typename decltype(tag)::Type;
    // For the common cases, we can just use a pointer to a static
    // OutputType.
    if constexpr (IsVector<T>::value) {
      return s_vecOutputType;
    } else if constexpr (IsOptional<T>::value) {
      return s_optOutputType;
    } else if constexpr (IsTuple<T>::value) {
      // Tuple output types are still static, but they're calculated
      // at compile time for each instantiation of the job.
      return std::apply(
        [] (auto&&... v) -> folly::Range<const OutputType*> {
          static std::array<OutputType, sizeof...(v)> a{std::move(v)...};
          return a;
        },
        typesToValues<T>(
          [] (size_t, auto tag) {
            using T2 = typename decltype(tag)::Type;
            if constexpr (IsVector<T2>::value) {
              return OutputType::Vec;
            } else if constexpr (IsOptional<T2>::value) {
              return OutputType::Opt;
            } else {
              static_assert(IsRef<T2>::value);
              return OutputType::Val;
            }
          }
        )
      );
    } else {
      static_assert(IsRef<T>::value);
      return s_valOutputType;
    }
  };

  using RetT = typename Job<C>::ReturnT;
  using FiniT = typename Job<C>::FiniT;

  constexpr bool hasFini =
    !std::is_void_v<typename Return<decltype(C::fini)>::type>;

  auto outputTypes = makeOutputTypes(Tag<RetT>{});
  auto finiTypes = hasFini
    ? HPHP::make_optional(makeOutputTypes(Tag<FiniT>{}))
    : std::nullopt;

  // Execute the job using the appropriate executor, receiving the
  // outputs as RefVals (with the format prescribed by the
  // OutputTypes).
  auto outputs = co_await folly::coro::co_invoke(
    [&] () -> folly::coro::Task<std::vector<RefValVec>> {
      co_return co_await tryWithThrottling<std::vector<RefValVec>>(
          [&]() -> folly::coro::Task<std::vector<RefValVec>> {
            // Note we calculate the RefVals here within the
            // lambda. This lets us move them into the exec call
            // (so we don't need to copy in the common case where
            // nothing fails).
            auto configRefVals = toRefVals(config);
            auto inputsRefVals = from(inputs)
              | mapped(toRefVals)
              | as<std::vector>();
            co_return co_await m_impl->exec(
              requestId,
              job.name(),
              std::move(configRefVals),
              std::move(inputsRefVals),
              outputTypes,
              finiTypes.get_pointer(),
              metadata
            );
          });
    }
  );

  // Map a RefVal for a particular output into a Ref<T>, where T is
  // the type corresponding to their output.
  auto const toRefSingle = [&] (RefVal&& v, auto tag)
    -> typename decltype(tag)::Type {
    using T = typename decltype(tag)::Type;
    if constexpr (IsVector<T>::value) {
      auto r = std::get_if<IdVec>(&v);
      assertx(r);
      return from(*r)
        | move
        | mapped([&] (RefId&& id) {
            return typename T::value_type{std::move(id)};
          })
        | as<std::vector>();
    } else if constexpr (IsOptional<T>::value) {
      auto r = std::get_if<Optional<RefId>>(&v);
      assertx(r);
      using Elem = std::remove_cv_t<
        std::remove_reference_t<decltype(*std::declval<T>())>
      >;
      static_assert(IsRef<Elem>::value);
      if (!r->has_value()) return std::nullopt;
      return Elem{std::move(**r)};
    } else {
      static_assert(IsRef<T>::value);
      auto r = std::get_if<RefId>(&v);
      assertx(r);
      return T{std::move(*r)};
    }
  };

  // The return type for the job can either be a single type, or a
  // tuple. This takes all the RefVals corresponding to the output and
  // maps them to that return type as appropriate.
  auto const toRef = [&] (RefValVec&& v, auto tag) {
    using T = typename decltype(tag)::Type;
    if constexpr (IsVector<T>::value ||
                  IsOptional<T>::value ||
                  IsRef<T>::value) {
      // Non-tuple. There should be only one output RefVal.
      assertx(v.size() == 1);
      return toRefSingle(std::move(v[0]), Tag<T>{});
    } else {
      // Otherwise there should be enough RefVals for the tuple
      // size. Build the tuple from converting the components.
      static_assert(IsTuple<T>::value);
      assertx(v.size() == std::tuple_size<T>{});
      return typesToValues<T>(
        [&] (size_t idx, auto tag) {
          assertx(idx < v.size());
          return toRefSingle(std::move(v[idx]), tag);
        }
      );
    }
  };

  auto const toRetT = [&] (RefValVec&& v) {
    return toRef(std::move(v), Tag<RetT>{});
  };

  if (metadata.optimistic && supportsOptimistic()) {
    ++m_stats->optimisticExecs;
  }

  if constexpr (hasFini) {
    assertx(outputs.size() == inputs.size() + 1);
    // Map over the outputs for each input set, and map them to Refs.
    auto out = inputs.empty()
      ? std::vector<typename ReturnRefs<C>::type>{}
      : seq(size_t{0}, inputs.size() - 1)
          | mapped([&] (size_t i) { return toRetT(std::move(outputs[i])); } )
          | as<std::vector>();
    auto fini = toRef(std::move(outputs.back()), Tag<FiniT>{});
    co_return std::make_tuple(std::move(out), std::move(fini));
  } else {
    assertx(outputs.size() == inputs.size());

    // Map over the outputs for each input set, and map them to Refs.
    auto out = from(outputs)
      | move
      | mapped(toRetT)
      | as<std::vector>();
    co_return out;
  }
}

// Turn the given blob into a value of type T.
template <typename T> T Client::unblobify(std::string&& blob) {
  static_assert(!detail::IsMarker<T>::value,
                "Special markers cannot be unblobified");
  if constexpr (std::is_same<T, std::string>::value) {
    return std::move(blob);
  } else {
    BlobDecoder decoder{blob.data(), blob.size()};
    return decoder.makeWhole<T>();
  }
}

// Turn the given value into a blob.
template <typename T> std::string Client::blobify(const T& t) {
  using BaseT =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  static_assert(!detail::IsMarker<BaseT>::value,
                "Special markers cannot be blobified");
  if constexpr (std::is_same<BaseT, std::string>::value) {
    return t;
  } else {
    BlobEncoder encoder;
    encoder(t);
    // It's a shame we have to copy this, but there's no way to
    // "attach" the BlobEncoder's buffer into the std::string without
    // copying.
    return std::string{(const char*)encoder.data(), encoder.size()};
  }
}

//////////////////////////////////////////////////////////////////////

// Run the given callable. If it throws Throttle, sleep for some
// amount of time, then retry. Repeat the process until the max number
// of retries is reached. Each retry will sleep longer.
template <typename T, typename F>
folly::coro::Task<T>
Client::Impl::tryWithThrottling(size_t retries,
                                std::chrono::milliseconds wait,
                                std::atomic<size_t>& throttles,
                                const F& f) {
  try {
    // If no retries, just run it normally
    if (retries == 0) co_return co_await f();
    for (size_t i = 0; i < retries-1; ++i) {
      try {
        co_return co_await f();
      } catch (const detail::Throttle&) {
        ++throttles;
        throttleSleep(i, wait);
      }
    }
    // The last retry will just let whatever throws escape, since we
    // won't rerun.
    co_return co_await f();
  } catch (const detail::Throttle& exn) {
    // Don't let a Throttle escape from here. Convert it to a normal
    // Error. This keeps us from getting nested throttles.
    throw Error{exn.what()};
  }
}

//////////////////////////////////////////////////////////////////////

template <typename K, typename V>
RefCache<K, V>::RefCache(Client& client)
  : m_client{client} {}

template <typename K, typename V>
folly::coro::Task<Ref<V>> RefCache<K, V>::get(const K& key,
                                              const V& val,
                                              folly::Executor::KeepAlive<> e) {
  // This indirection avoids copying val unless we actually need to.
  return m_map.get(
    key,
    [&] { return [this, val] { return m_client.store(std::move(val)); }; },
    std::move(e)
  );
}

//////////////////////////////////////////////////////////////////////

}
