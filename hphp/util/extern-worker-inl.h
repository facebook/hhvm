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
#include <folly/portability/Filesystem.h>

#include <boost/filesystem.hpp>

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
void Job<C>::init(const folly::fs::path& root) const {
  using namespace detail;
  using Args = typename Params<decltype(C::init)>::type;

  // For each expected input, load the file, and deserialize it into
  // the appropriate type. Return the types as a tuple, which can then
  // std::apply to C::init, passing the inputs.
  std::apply(
    C::init,
    typesToValues<Args>(
      [&] (size_t idx, auto tag) {
        return deserialize<typename decltype(tag)::Type>(
          root / folly::to<std::string>(idx)
        );
      }
    )
  );

  using Ret = typename Return<decltype(C::init)>::type;
  static_assert(std::is_void_v<Ret>, "init() must return void");
}

template <typename C>
void Job<C>::fini() const {
  using Ret = typename detail::Return<decltype(C::fini)>::type;
  static_assert(std::is_void_v<Ret>, "fini() must return void");
  // fini() is easy since it doesn't receive nor return anything.
  C::fini();
}

template <typename C>
void Job<C>::run(const folly::fs::path& inputRoot,
                 const folly::fs::path& outputRoot) const {
  using namespace detail;

  // For each expected input, load the file, and deserialize it into
  // the appropriate type, turning all of the types into a tuple.
  using Args = typename Params<decltype(C::run)>::type;
  auto inputs = time(
    "loading inputs",
    [&] {
      return typesToValues<Args>(
        [&] (size_t idx, auto tag) {
          return deserialize<typename decltype(tag)::Type>(
            inputRoot / folly::to<std::string>(idx)
          );
        }
      );
    }
  );

  // Apply the tuple to C::run, passing the types as parameters.
  auto outputs = time(
    "actual run",
    [&] { return std::apply(C::run, std::move(inputs)); }
  );

  using Ret = typename Return<decltype(C::run)>::type;
  static_assert(!std::is_void_v<Ret>, "run() must return something");

  // Serialize the outputs into the output directory.
  time("writing outputs", [&] { return serialize(outputs, 0, outputRoot); });
}

//////////////////////////////////////////////////////////////////////

namespace detail {

//////////////////////////////////////////////////////////////////////

// Given a file path, load the contents of the file, deserialize them
// into the type T, and return it.
template <typename T>
T JobBase::deserialize(const folly::fs::path& path) {
  using namespace detail;
  if constexpr (std::is_same<T, std::string>::value) {
    // A std::string is always stored as itself (this lets us store
    // files as their contents without having to encode them).
    return readFile(path);
  } else if constexpr (IsVariadic<T>::value) {
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    // Variadic<T> is actually a directory, not a file. Recurse into
    // it, and do the deserialization for every file within it.
    T out;
    for (size_t i = 0;; ++i) {
      auto const valPath = path / folly::to<std::string>(i);
      // A break in the numbering means the end of the vector.
      if (!folly::fs::exists(valPath)) break;
      out.vals.emplace_back(deserialize<typename T::Type>(valPath));
    }
    return out;
  } else if constexpr (IsOpt<T>::value) {
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    // Opt<T> is like T, except the file may not exist (so is nullopt
    // otherwise).
    T out;
    if (folly::fs::exists(path)) {
      out.val.emplace(deserialize<typename T::Type>(path));
    }
    return out;
  } else {
    // For most types, the data is encoded using BlobEncoder, so undo
    // that.
    static_assert(!IsMulti<T>::value, "Multi can only be used as return type");
    auto const data = readFile(path);
    BlobDecoder decoder{data.data(), data.size(), false};
    return decoder.makeWhole<T>();
  }
}

// Given a value, an index of that value (its positive in the output
// values), and an output root, serialize the value, and write its
// contents to the appropriate file.
template <typename T>
void JobBase::serialize(const T& v,
                        size_t idx,
                        const folly::fs::path& root) {
  using namespace detail;
  if constexpr (std::is_same<T, std::string>::value) {
    // std::string isn't serialized, but written as itself as
    // root/idx.
    return writeFile(root / folly::to<std::string>(idx), v.data(), v.size());
  } else if constexpr (IsVariadic<T>::value) {
    // For Variadic<T>, we create a directory root/idx, and under it,
    // write a file for every element in the vector.
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    auto const path = root / folly::to<std::string>(idx);
    folly::fs::create_directory(path, root);
    for (size_t i = 0; i < v.vals.size(); ++i) {
      serialize(v.vals[i], i, path);
    }
  } else if constexpr (IsOpt<T>::value) {
    // Opt<T> is like T, except nothing is written if the value isn't
    // present.
    static_assert(!IsMarker<typename T::Type>::value,
                  "Special markers cannot be nested");
    if (!v.val.has_value()) return;
    serialize(*v.val, idx, root);
  } else if constexpr (IsMulti<T>::value) {
    // Treat Multi as equivalent to std::tuple (IE, write each element
    // to a separate file).
    assertx(idx == 0);
    for_each(
      v.vals,
      [&] (auto const& elem, size_t tupleIdx) {
        static_assert(
          !IsMulti<
            std::remove_cv_t<std::remove_reference_t<decltype(elem)>>
          >::value,
         "Multi cannot be nested"
        );
        serialize(elem, tupleIdx, root);
      }
    );
  } else {
    // Most types are just encoded with BlobEncoder and written as
    // root/idx
    BlobEncoder encoder{false};
    encoder(v);
    writeFile(
      root / folly::to<std::string>(idx),
      (const char*)encoder.data(), encoder.size()
    );
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

inline RefId::RefId(std::string id, size_t size)
  : m_id{std::move(id)}, m_size{size}
{}

inline bool RefId::operator==(const RefId& o) const {
  return std::tie(m_id, m_size) == std::tie(o.m_id, o.m_size);
}

inline bool RefId::operator!=(const RefId& o) const {
  return !(*this == o);
}

inline std::string RefId::toString() const {
  return folly::sformat("{}:{}", m_id, m_size);
}

//////////////////////////////////////////////////////////////////////

template <typename T>
inline Ref<T>::Ref(RefId id, bool fromFallback)
  : m_id{std::move(id)}
  , m_fromFallback{fromFallback}
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

//////////////////////////////////////////////////////////////////////

inline const std::string& Client::implName() const {
  return m_impl->name();
}

inline bool Client::usingSubprocess() const {
  return m_impl->isSubprocess();
}

// Run the given callable F with the normal implementation. If it
// throws an Error, set didFallback to true, and attempt to re-run F
// with the fallback implementation.
template <typename T, typename F>
coro::Task<T> Client::tryWithFallback(F f, bool& didFallback) {
  // If we're forcing fallbacks, or if the main implementation has
  // disabled itself, go straight to the fallback case.
  if (!m_forceFallback && !m_impl->isDisabled()) {
    try {
      // Attempt the normal implementation.
      didFallback = false;
      HPHP_CORO_RETURN(HPHP_CORO_AWAIT(f(*m_impl, false)));
    } catch (const Error& exn) {
      // It failed. If the main implementation *is* the subprocess
      // implementation, there's no point in trying again. Just
      // rethrow the error. Likewise, if Options has determined we
      // shouldn't use the subprocess implementaiton, rethrow the
      // error.
      if (m_impl->isSubprocess()) throw;
      if (m_options.m_useSubprocess == Options::UseSubprocess::Never) throw;
      // Check this again. The implementation could have disabled
      // itself while using it above, and in that case, its the
      // implementation's responsibility to print something to the
      // log.
      if (!m_impl->isDisabled()) {
        Logger::FError(
          "Error: \"{}\". Attempting to retry with local fallback...",
          exn.what()
        );
      }
    }
  }
  // Fallback case. Make a fallback implementation if we haven't
  // already (LockFreeLazy will ensure this), and call it. If it
  // throws, nothing to be done, it will be propagated to the caller.
  didFallback = true;
  auto& fallback = *m_fallbackImpl.get([this] { return makeFallbackImpl(); });
  HPHP_CORO_RETURN(HPHP_CORO_AWAIT(f(fallback, true)));
}

template <typename T>
coro::Task<T> Client::load(Ref<T> r) {
  RequestId requestId{"load blob"};

  // Get the appropriate implementation (it could have been created by
  // a fallback implementation), and forward the request to it.
  auto& impl = r.m_fromFallback ? *m_fallbackImpl.rawGet() : *m_impl;
  auto result = HPHP_CORO_AWAIT(impl.load(requestId, IdVec{std::move(r.m_id)}));
  assertx(result.size() == 1);
  FTRACE(4, "{} blob is {} bytes\n",
         requestId.tracePrefix(), result[0].size());
  HPHP_CORO_RETURN(unblobify<T>(std::move(result[0])));
}

template <typename T, typename... Ts>
coro::Task<std::tuple<T, Ts...>> Client::load(Ref<T> r, Ref<Ts>... rs) {
  using namespace detail;

  RequestId requestId{"load blobs"};
  FTRACE(2, "{} {} blobs requested\n",
         requestId.tracePrefix(), sizeof...(Ts) + 1);

  // Retrieve the ids from the Refs, and split them into a set from
  // the main implementation, and a set from the fallback
  // implementation.
  IdVec main;
  IdVec fallback;
  // Keep the indices of the original list, so we can restore it
  // afterwards. If a mapping isn't present,it's an identity mapping
  // for the non-fallback implementation (which conveniently means we
  // don't need to store anything for the common case of there being
  // no fallbacks).
  hphp_fast_map<size_t, std::pair<bool, size_t>> indexMappings;
  for_each(
    std::make_tuple(std::move(r), std::move(rs)...),
    [&] (auto&& r, size_t idx) {
      if (r.m_fromFallback) {
        indexMappings.emplace(idx, std::make_pair(true, fallback.size()));
        fallback.emplace_back(std::move(r.m_id));
      } else {
        if (idx != main.size()) {
          indexMappings.emplace(idx, std::make_pair(false, main.size()));
        }
        main.emplace_back(std::move(r.m_id));
      }
    }
  );

  auto const DEBUG_ONLY mainSize = main.size();
  auto const DEBUG_ONLY fallbackSize = fallback.size();
  assertx(mainSize + fallbackSize == sizeof...(Ts) + 1);

  // Do the loads from either implementation (in the normal case, only
  // one of these will be executed).
  auto mainBlobs = !main.empty()
    ? HPHP_CORO_AWAIT(m_impl->load(requestId, std::move(main)))
    : BlobVec{};
  auto fallbackBlobs = !fallback.empty()
    ? HPHP_CORO_AWAIT(
        m_fallbackImpl.rawGet()->load(requestId, std::move(fallback))
      )
    : BlobVec{};

  assertx(mainBlobs.size() == mainSize);
  assertx(fallbackBlobs.size() == fallbackSize);
  using OutTuple = std::tuple<T, Ts...>;

  // Now restore the results back into the output tuple.
  auto ret = typesToValues<OutTuple>(
    [&] (size_t idx, auto tag) {
      assertx(idx < mainBlobs.size() + fallbackBlobs.size());

      // For tuple position idx, use indexMappings to figure out which
      // of mainBlobs or fallbackBlobs entry corresponds to this
      // entry.
      auto& blob = [&] () -> std::string& {
        // In the common case, we'll have no mappings, and idx will be
        // an identity mapping into mainBlobs.
        if (indexMappings.empty()) {
          assertx(idx < mainBlobs.size());
          return mainBlobs[idx];
        }
        // Otherwise do the lookup.
        auto const it = indexMappings.find(idx);
        if (it == indexMappings.end()) {
          // No entry means idx maps to idx in mainBlobs.
          assertx(idx < mainBlobs.size());
          return mainBlobs[idx];
        }
        // Otherwise use the entry to find the right blob.
        if (it->second.first) {
          assertx(it->second.second < fallbackBlobs.size());
          return fallbackBlobs[it->second.second];
        } else {
          assertx(it->second.second < mainBlobs.size());
          return mainBlobs[it->second.second];
        }
      }();

      FTRACE(4, "{} blob #{} is {} bytes\n",
             requestId.tracePrefix(), idx, blob.size());
      // Turn it into the value.
      return unblobify<typename decltype(tag)::Type>(std::move(blob));
    }
  );
  HPHP_CORO_MOVE_RETURN(ret);
}

template <typename T>
coro::Task<std::vector<T>> Client::load(std::vector<Ref<T>> rs) {
  using namespace folly::gen;

  RequestId requestId{"load blobs"};
  FTRACE(2, "{} {} blobs requested\n",
         requestId.tracePrefix(), rs.size());

  // Retrieve the RefId from the refs and split them into the ones
  // which come from the main implementation and the fallback
  // implementation.
  IdVec main;
  IdVec fallback;
  for (auto& r : rs) {
    if (r.m_fromFallback) {
      fallback.emplace_back(std::move(r.m_id));
    } else {
      main.emplace_back(std::move(r.m_id));
    }
  }

  auto const DEBUG_ONLY mainSize = main.size();
  auto const DEBUG_ONLY fallbackSize = fallback.size();
  assertx(mainSize + fallbackSize == rs.size());

  // Do the loads from either implementation (in the normal case, only
  // one of these will be executed).
  auto mainBlobs = !main.empty()
    ? HPHP_CORO_AWAIT(m_impl->load(requestId, std::move(main)))
    : BlobVec{};
  auto fallbackBlobs = !fallback.empty()
    ? HPHP_CORO_AWAIT(
        m_fallbackImpl.rawGet()->load(requestId, std::move(fallback))
      )
    : BlobVec{};

  assertx(mainBlobs.size() == mainSize);
  assertx(fallbackBlobs.size() == fallbackSize);

  std::vector<T> out;
  out.reserve(rs.size());

  // Stitch together mainBlobs and fallbackBlobs back together in the
  // same order their Refs were passed to this function. We can
  // reconstruct the order using the input Ref vector.
  size_t mainIdx = 0;
  size_t fallbackIdx = 0;
  for (auto const& r : rs) {
    auto& blob = [&] () -> std::string& {
      if (r.m_fromFallback) {
        assertx(fallbackIdx < fallbackBlobs.size());
        return fallbackBlobs[fallbackIdx++];
      } else {
        assertx(mainIdx < mainBlobs.size());
        return mainBlobs[mainIdx++];
      }
    }();
    FTRACE(4, "{} blob #{} is {} bytes\n",
           requestId.tracePrefix(), out.size(), blob.size());
    out.emplace_back(unblobify<T>(std::move(blob)));
  }
  assertx(out.size() == rs.size());
  assertx(mainIdx == mainBlobs.size());
  assertx(fallbackIdx == fallbackBlobs.size());

  HPHP_CORO_MOVE_RETURN(out);
}

template <typename T, typename... Ts>
coro::Task<std::vector<std::tuple<T, Ts...>>>
Client::load(std::vector<std::tuple<Ref<T>, Ref<Ts>...>> rs) {
  using namespace detail;
  using namespace folly::gen;

  using OutTuple = std::tuple<T, Ts...>;
  auto constexpr tupleSize = std::tuple_size<OutTuple>{};

  RequestId requestId{"load blobs"};
  FTRACE(2, "{} {} blobs requested\n",
         requestId.tracePrefix(), rs.size() * tupleSize);

  // This is a hybrid of the variadic case and the vector case:

  // Retrieve the RefId from the refs and split them into the ones
  // which come from the main implementation and the fallback
  // implementation. "Flatten" out the refs into flat lists,
  // disregarding the tuple structure (which will be rebuilt later).
  IdVec main;
  IdVec fallback;
  hphp_fast_map<size_t, std::pair<bool, size_t>> indexMappings;
  for (size_t i = 0; i < rs.size(); ++i) {
    for_each(
      std::move(rs[i]),
      [&] (auto&& r, size_t tupleIdx) {
        auto const idx = i * tupleSize + tupleIdx;
        if (r.m_fromFallback) {
          indexMappings.emplace(idx, std::make_pair(true, fallback.size()));
          fallback.emplace_back(std::move(r.m_id));
        } else {
          if (idx != main.size()) {
            indexMappings.emplace(idx, std::make_pair(false, main.size()));
          }
          main.emplace_back(std::move(r.m_id));
        }
      }
    );
  }

  auto const DEBUG_ONLY mainSize = main.size();
  auto const DEBUG_ONLY fallbackSize = fallback.size();
  assertx(mainSize + fallbackSize == rs.size() * tupleSize);

  // Do the loads from either implementation (in the normal case, only
  // one of these will be executed).
  auto mainBlobs = !main.empty()
    ? HPHP_CORO_AWAIT(m_impl->load(requestId, std::move(main)))
    : BlobVec{};
  auto fallbackBlobs = !fallback.empty()
    ? HPHP_CORO_AWAIT(
        m_fallbackImpl.rawGet()->load(requestId, std::move(fallback))
      )
    : BlobVec{};

  assertx(mainBlobs.size() == mainSize);
  assertx(fallbackBlobs.size() == fallbackSize);

  // Now combine mainBlobs and fallbackBlobs back together, in the
  // same order as their associated Refs were given (also restoring
  // the tuple structure of the inputs).
  std::vector<OutTuple> out;
  out.reserve(rs.size());

  for (size_t i = 0; i < rs.size(); ++i) {
    out.emplace_back(
      typesToValues<OutTuple>(
        [&] (size_t tupleIdx, auto tag) {
          // This is similar to logic for load(Ref<T>, Ref<T2>, ...)
          auto const idx = i * tupleSize + tupleIdx;
          assertx(idx < mainBlobs.size() + fallbackBlobs.size());

          auto& blob = [&] () -> std::string& {
            if (indexMappings.empty()) {
              assertx(idx < mainBlobs.size());
              return mainBlobs[idx];
            }
            auto const it = indexMappings.find(idx);
            if (it == indexMappings.end()) {
              assertx(idx < mainBlobs.size());
              return mainBlobs[idx];
            }
            if (it->second.first) {
              assertx(it->second.second < fallbackBlobs.size());
              return fallbackBlobs[it->second.second];
            } else {
              assertx(it->second.second < mainBlobs.size());
              return mainBlobs[it->second.second];
            }
          }();

          FTRACE(4, "{} blob #{} is {} bytes\n",
                 requestId.tracePrefix(), idx, blob.size());
          return unblobify<typename decltype(tag)::Type>(std::move(blob));
        }
      )
    );
  }
  assertx(out.size() == rs.size());

  HPHP_CORO_MOVE_RETURN(out);
}

// All of the store functions are similar: Serialize the inputs into
// blobs, call (with fallback) the store function on the
// implementation, then wrap the output RefIds into Refs with the
// appropriate types.

template <typename T>
coro::Task<Ref<T>> Client::store(T t) {
  RequestId requestId{"store blob"};

  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool) {
      auto blob = blobify(t);
      FTRACE(2, "{} blob is {} bytes\n", requestId.tracePrefix(), blob.size());
      return i.store(requestId, {}, BlobVec{std::move(blob)}, nullptr, nullptr);
    },
    wasFallback
  ));
  assertx(ids.size() == 1);

  Ref<T> ref{std::move(ids[0]), wasFallback};
  HPHP_CORO_MOVE_RETURN(ref);
}

template <typename T, typename... Ts>
coro::Task<std::tuple<Ref<T>, Ref<Ts>...>> Client::store(T t, Ts... ts) {
  using namespace detail;
  RequestId requestId{"store blobs"};

  FTRACE(2, "{} storing {} blobs\n",
         requestId.tracePrefix(), sizeof...(Ts) + 1);

  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool) {
      BlobVec blobs{{ blobify(t), blobify(ts)... }};
      ONTRACE(4, [&] {
        for (auto const& b : blobs) {
          FTRACE(4, "{} storing {} byte blob\n",
                 requestId.tracePrefix(), b.size());
        }
      }());
      return i.store(requestId, {}, std::move(blobs), nullptr, nullptr);
    },
    wasFallback
  ));
  assertx(ids.size() == sizeof...(Ts) + 1);

  using OutTuple = std::tuple<T, Ts...>;
  auto ret = typesToValues<OutTuple>(
    [&] (size_t idx, auto tag) {
      assertx(idx < ids.size());
      return Ref<typename decltype(tag)::Type>{
        std::move(ids[idx]), wasFallback
      };
    }
  );
  HPHP_CORO_MOVE_RETURN(ret);
}

template <typename T>
coro::Task<std::vector<Ref<T>>> Client::storeMulti(std::vector<T> ts) {
  using namespace folly::gen;

  RequestId requestId{"store blobs"};

  FTRACE(2, "{} storing {} blobs\n",
         requestId.tracePrefix(), ts.size());

  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool) {
      auto blobs = from(ts)
        | mapped([&] (const T& t) {
            auto blob = blobify(t);
            FTRACE(4, "{} storing {} byte blob\n",
                   requestId.tracePrefix(), blob.size());
            return blob;
          })
        | as<std::vector>();
      assertx(blobs.size() == ts.size());
      return i.store(requestId, {}, std::move(blobs), nullptr, nullptr);
    },
    wasFallback
  ));
  assertx(ids.size() == ts.size());

  auto out = from(ids)
    | move
    | mapped([&] (RefId&& id) { return Ref<T>{std::move(id), wasFallback}; })
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

template <typename T, typename... Ts>
coro::Task<std::vector<std::tuple<Ref<T>, Ref<Ts>...>>>
Client::storeMultiTuple(std::vector<std::tuple<T, Ts...>> ts) {
  using namespace folly::gen;
  using namespace detail;

  RequestId requestId{"store blobs"};

  using OutTuple = std::tuple<T, Ts...>;
  auto constexpr tupleSize = std::tuple_size<OutTuple>{};

  FTRACE(2, "{} storing {} blobs\n",
         requestId.tracePrefix(), ts.size() * tupleSize);

  auto wasFallback = false;
  auto ids = HPHP_CORO_AWAIT(tryWithFallback<IdVec>(
    [&] (Impl& i, bool) {
      // Map each tuple to a vector of RefIds, then concat all of the
      // vectors together to get one flat list.
      auto blobs = from(ts)
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
      assertx(blobs.size() == ts.size() * tupleSize);
      return i.store(requestId, {}, std::move(blobs), nullptr, nullptr);
    },
    wasFallback
  ));
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
            return Ref<typename decltype(tag)::Type>{
              std::move(ids[idx]), wasFallback
            };
          }
        );
      })
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

template <typename C> coro::Task<std::vector<typename Job<C>::ReturnT>>
Client::exec(const Job<C>& job,
             typename Job<C>::ConfigT config,
             std::vector<typename Job<C>::InputsT> inputs,
             bool* cached) {
  using namespace folly::gen;
  using namespace detail;

  RequestId requestId{"exec"};
  FTRACE(2, "{} executing \"{}\" ({} runs)\n",
         requestId.tracePrefix(), job.name(),
         inputs.size());

  // Return true if a Ref (or some container of them allowed as
  // inputs) came from the fallback implementation. If so, we'll force
  // everything to come from the fallback implementation (loading and
  // storing as required), and exec on the fallback
  // implementation. One fallback Ref "poisons" everything.
  auto const isFallback = [] (auto const& r) {
    using Type = std::remove_cv_t<std::remove_reference_t<decltype(r)>>;
    if constexpr (IsVector<Type>::value) {
      for (auto const& e : r) {
        if (e.m_fromFallback) return true;
      }
      return false;
    } else if constexpr (IsOptional<Type>::value) {
      if (!r.has_value()) return false;
      return r->m_fromFallback;
    } else {
      static_assert(IsRef<Type>::value);
      return r.m_fromFallback;
    }
  };
  auto const tupleIsFallback = [&] (auto const& tuple) {
    auto f = false;
    for_each(tuple, [&] (auto const& r) { f |= isFallback(r); });
    return f;
  };
  auto const vecIsFallback = [&] (auto const& v) {
    return std::any_of(v.begin(), v.end(), tupleIsFallback);
  };

  // Make all of the inputs come from the fallback
  // implementation. This is a lambda because we might have to do it
  // two different places.
  auto const makeAllFallback = [&] () -> coro::Task<void> {
    // Vector of all RefIds which come from the non-fallback
    // implementation.
    IdVec ids;

    // For every input, if the Ref doesn't come from the fallback
    // implementation, add it to "ids".
    auto const addNonFallback = [&] (auto const& r) {
      using Type = std::remove_cv_t<std::remove_reference_t<decltype(r)>>;
      if constexpr (IsVector<Type>::value) {
        for (auto const& e : r) {
          if (e.m_fromFallback) continue;
          ids.emplace_back(e.m_id);
        }
      } else if constexpr (IsOptional<Type>::value) {
        if (r.has_value() && !r->m_fromFallback) {
          ids.emplace_back(r->m_id);
        }
      } else {
        static_assert(IsRef<Type>::value);
        if (!r.m_fromFallback) ids.emplace_back(r.m_id);
      }
    };
    for_each(config, addNonFallback);
    for (auto const& t : inputs) for_each(t, addNonFallback);

    // If there's nothing there, everything is already from the
    // fallback implementation, so we're done.
    if (ids.empty()) HPHP_CORO_RETURN_VOID;

    // Otherwise load just those from the non-fallback implementation
    // (if this fails, there's nothing we can do).
    auto const DEBUG_ONLY size = ids.size();
    auto blobs = HPHP_CORO_AWAIT(m_impl->load(requestId, std::move(ids)));
    assertx(blobs.size() == size);

    // Then store them with the fallback implementation.
    auto stores = HPHP_CORO_AWAIT(
      m_fallbackImpl.rawGet()->store(
        requestId,
        {},
        std::move(blobs),
        nullptr,
        nullptr
      )
    );
    assertx(stores.size() == size);

    // Iterate over all the inputs again. If the input came from the
    // non-fallback implementation, then overwrite (in place) it's
    // RefId with the new RefId we got from storing it in the fallback
    // implementation.
    size_t idx = 0;
    auto const setToFallback = [&] (auto& r) {
      using Type = std::remove_cv_t<std::remove_reference_t<decltype(r)>>;
      if constexpr (IsVector<Type>::value) {
        for (auto& e : r) {
          if (e.m_fromFallback) continue;
          assertx(idx < stores.size());
          e.m_fromFallback = true;
          e.m_id = std::move(stores[idx++]);
        }
      } else if constexpr (IsOptional<Type>::value) {
        if (r.has_value() && !r->m_fromFallback) {
          assertx(idx < stores.size());
          r->m_fromFallback = true;
          r->m_id = std::move(stores[idx++]);
        }
        } else {
        static_assert(IsRef<Type>::value);
        if (!r.m_fromFallback) {
          assertx(idx < stores.size());
          r.m_fromFallback = true;
          r.m_id = std::move(stores[idx++]);
        }
      }
    };
    for_each(config, setToFallback);
    for (auto& t : inputs) for_each(t, setToFallback);
    assertx(idx == stores.size());

    // At this point, all Refs should be from the fallback
    // implementation.
    HPHP_CORO_RETURN_VOID;
  };

  // Do we need to execute on the fallback implementation? If the
  // fallback implementation isn't created, then obviously not (common
  // case). Otherwise, if the main implementation is disabled, we're
  // forced to. If any input RefId is from the fallback
  // implementation, we're also forced to.
  auto useFallback = false;
  if (m_fallbackImpl.present() &&
      (m_impl->isDisabled() ||
       tupleIsFallback(config) ||
       vecIsFallback(inputs))) {
    // If we're executing on the fallback implementation, all inputs
    // need to be from there.
    HPHP_CORO_AWAIT(makeAllFallback());
    useFallback = true;
  }

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

  using RetT = typename Job<C>::ReturnT;

  // OutputType is similar to RefVals, but for outputs. Since there's
  // no RefId (yet) for the outputs, we can use a simple enum to
  // describe the output format.
  auto const outputTypes = [&] () -> folly::Range<const OutputType*> {
    // For the common cases, we can just use a pointer to a static
    // OutputType.
    if constexpr (IsVector<RetT>::value) {
      return s_vecOutputType;
    } else if constexpr (IsOptional<RetT>::value) {
      return s_optOutputType;
    } else if constexpr (IsTuple<RetT>::value) {
      // Tuple output types are still static, but they're calculated
      // at compile time for each instantiation of the job.
      return std::apply(
        [] (auto&&... v) -> folly::Range<const OutputType*> {
          static std::array<OutputType, sizeof...(v)> a{std::move(v)...};
          return a;
        },
        typesToValues<RetT>(
          [] (size_t, auto tag) {
            using T = typename decltype(tag)::Type;
            if constexpr (IsVector<T>::value) {
              return OutputType::Vec;
            } else if constexpr (IsOptional<T>::value) {
              return OutputType::Opt;
            } else {
              static_assert(IsRef<T>::value);
              return OutputType::Val;
            }
          }
        )
      );
    } else {
      static_assert(IsRef<RetT>::value);
      return s_valOutputType;
    }
  }();

  // Execute the job using the appropriate executor, receiving the
  // outputs as RefVals (with the format prescribed by the
  // OutputTypes).
  auto outputs = HPHP_CORO_AWAIT(coro::invoke(
    [&] () -> coro::Task<std::vector<RefValVec>> {
      if (useFallback) {
        auto configRefVals = toRefVals(config);
        auto inputsRefVals = from(inputs)
          | mapped(toRefVals)
          | as<std::vector>();
        HPHP_CORO_RETURN(
          HPHP_CORO_AWAIT(
            m_fallbackImpl.rawGet()->exec(
              requestId,
              job.name(),
              std::move(configRefVals),
              std::move(inputsRefVals),
              outputTypes,
              cached
            )
          )
        );
      } else {
        // Not using the fallback executor. Use tryWithFallback since
        // the execution can fail.
        HPHP_CORO_RETURN(
          HPHP_CORO_AWAIT(
            tryWithFallback<std::vector<RefValVec>>(
              [&] (Impl& i, bool fallback)
              -> coro::Task<std::vector<RefValVec>> {
                // If we've failed and we're now trying the fallback,
                // we need to make all of the inputs be from the
                // fallback executor.
                if (fallback) HPHP_CORO_AWAIT(makeAllFallback());
                // Note we calculate the RefVals here within the
                // lambda. This lets us move them into the exec call
                // (so we don't need to copy in the common case where
                // nothing fails).
                auto configRefVals = toRefVals(config);
                auto inputsRefVals = from(inputs)
                  | mapped(toRefVals)
                  | as<std::vector>();
                HPHP_CORO_RETURN(HPHP_CORO_AWAIT(i.exec(
                  requestId,
                  job.name(),
                  std::move(configRefVals),
                  std::move(inputsRefVals),
                  outputTypes,
                  cached
                )));
              },
              useFallback
            )
          )
        );
      }
    }
  ));
  assertx(outputs.size() == inputs.size());

  // Map a RefVal for a particular output into a Ref<T>, where T is
  // the type corresponding to their output.
  auto const toRetTSingle = [&] (RefVal&& v, auto tag)
    -> typename decltype(tag)::Type {
    using T = typename decltype(tag)::Type;
    if constexpr (IsVector<T>::value) {
      auto r = boost::get<IdVec>(&v);
      assertx(r);
      return from(*r)
        | move
        | mapped([&] (RefId&& id) {
            return typename T::value_type{std::move(id), useFallback};
          })
        | as<std::vector>();
    } else if constexpr (IsOptional<T>::value) {
      auto r = boost::get<Optional<RefId>>(&v);
      assertx(r);
      using Elem = std::remove_cv_t<
        std::remove_reference_t<decltype(*std::declval<T>())>
      >;
      static_assert(IsRef<Elem>::value);
      if (!r->has_value()) return std::nullopt;
      return Elem{std::move(**r), useFallback};
    } else {
      static_assert(IsRef<T>::value);
      auto r = boost::get<RefId>(&v);
      assertx(r);
      return T{std::move(*r), useFallback};
    }
  };

  // The return type for the job can either be a single type, or a
  // tuple. This takes all the RefVals corresponding to the output and
  // maps them to that return type as appropriate.
  auto const toRetT = [&] (RefValVec&& v) -> RetT {
    if constexpr (IsVector<RetT>::value ||
                  IsOptional<RetT>::value ||
                  IsRef<RetT>::value) {
      // Non-tuple. There should be only one output RefVal.
      assertx(v.size() == 1);
      return toRetTSingle(std::move(v[0]), Tag<RetT>{});
    } else {
      // Otherwise there should be enough RefVals for the tuple
      // size. Build the tuple from converting the components.
      static_assert(IsTuple<RetT>::value);
      assertx(v.size() == std::tuple_size<RetT>{});
      return typesToValues<RetT>(
        [&] (size_t idx, auto tag) {
          assertx(idx < v.size());
          return toRetTSingle(std::move(v[idx]), tag);
        }
      );
    }
  };

  // Map over the outputs for each input set, and map them to Refs.
  auto out = from(outputs)
    | move
    | mapped(toRetT)
    | as<std::vector>();
  HPHP_CORO_MOVE_RETURN(out);
}

// Turn the given blob into a value of type T.
template <typename T> T Client::unblobify(std::string&& blob) {
  static_assert(!detail::IsMarker<T>::value,
                "Special markers cannot be unblobified");
  if constexpr (std::is_same<T, std::string>::value) {
    return std::move(blob);
  } else {
    BlobDecoder decoder{blob.data(), blob.size(), false};
    return decoder.makeWhole<T>();
  }
}

// Turn the given value into a blob.
template <typename T> std::string Client::blobify(T&& t) {
  using BaseT =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  static_assert(!detail::IsMarker<BaseT>::value,
                "Special markers cannot be blobified");
  if constexpr (std::is_same<BaseT, std::string>::value) {
    return std::forward<T>(t);
  } else {
    BlobEncoder encoder{false};
    encoder(t);
    // It's a shame we have to copy this, but there's no way to
    // "attach" the BlobEncoder's buffer into the std::string without
    // copying.
    return std::string{(const char*)encoder.data(), encoder.size()};
  }
}

//////////////////////////////////////////////////////////////////////

template <typename K, typename V>
RefCache<K, V>::RefCache(Client& client)
  : m_client{client} {}

template <typename K, typename V>
coro::Task<Ref<V>> RefCache<K, V>::get(const K& key,
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
