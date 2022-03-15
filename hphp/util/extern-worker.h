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

#pragma once

#include "hphp/util/coro.h"
#include "hphp/util/optional.h"
#include "hphp/util/trace.h"

#include <folly/portability/Filesystem.h>

#include <string>
#include <vector>

#include <boost/variant.hpp>

/*
 * Framework for executing work outside of the HHVM process.
 *
 * The main use case for this framework is to provide a mechanism to
 * execute work in a distributed manner on remote machines. This can
 * allow for greater parallelism than just running locally, and allow
 * for datasets which exceed the memory available on any one
 * machine. However, it has been designed to be agnostic (as much as
 * possible) as to the exact mechanism used for execution, meaning it
 * can be expanded for other use cases we forsee in the future.
 *
 * Terminology:
 *
 * "Job" - Encapsulate some piece of work. A Job has a typed set of
 * inputs, and produces a typed set of outputs. Multiple instances of
 * the same job can be run in one "execution" (with different
 * inputs). A job also has a separate set of "init" inputs, which are
 * run once per execution (used for global initialization). Exactly
 * where/how the job "executes" is not specified (depends on
 * implementation and config), but it will be outside of the
 * process. Some implementations may cache the results of an
 * execution, meaning that it can produce the output without having to
 * actually run the job.
 *
 * "Ref" - Refs represent some piece of data. You do not provide
 * inputs or read outputs from a job directly. Instead these are
 * represented by refs to the data. You "store" a piece of data to
 * obtain its ref (which can then be provided to a job
 * execution). Likewise, given a ref, you can "load" it to obtain the
 * data. This lets you feed data from one job to the next without
 * having to explicitly load it. Refs are type-safe. They know which
 * type of data they point to, meaning you know (at compile time) that
 * you're passing the right kind of data to a job). Refs contain a
 * "RefId", which is a string/size pair. This uniquely identifies the
 * data, but the meaning of the pair is up to the implementation. Any
 * data stored needs to be serializable with BlobEncoder/BlobDecoder.
 *
 * "Markers" - In some situations, you want to be able to represent a
 * variadic list of data, or an optional piece of data. You could use
 * std::vector or Optional for this, but you would get a
 * Ref<std::vector>, which means a ref to a std::vector. It might be
 * more useful to have a std::vector<Ref>, which means you know how
 * many refs were produced without having to load everything. A
 * special set of "marker" types can be used for this. They're
 * "Variadic", "Opt", and "Multi", which have similar meanings as
 * std::vector, Optional, and std::tuple, but are new types to avoid
 * ambiguity. These can only be used as input/outputs of jobs. Multi
 * can only be used as a return type.
 *
 * "Client" - Represents an instance of a extern-worker
 * framework. Responsible for producing/consuming refs, and executing
 * jobs. A client is backed by a particular implementation, which does
 * the actual work. The default implementation uses fork+exec, but
 * others can be provided by a hook mechanism.
 *
 * To use the extern-worker framework, you must provide a handler in
 * your main() function. If passed extern_worker::s_option as argv[1],
 * call extern_worker::main(), passing in the argv and argc.
 */

//////////////////////////////////////////////////////////////////////

// Implementation details to avoid cluttering interface
#define incl_HPHP_EXTERN_WORKER_DETAIL_H_
#include "hphp/util/extern-worker-detail.h"
#undef incl_HPHP_EXTERN_WORKER_DETAIL_H_

//////////////////////////////////////////////////////////////////////

namespace HPHP::extern_worker {

//////////////////////////////////////////////////////////////////////

extern const char* const s_option;

// Entry point for workers
extern int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

// Thrown by any of extern-worker functions to indicate an error
struct Error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

//////////////////////////////////////////////////////////////////////

/*
 * Represents a job which can be executed. The implementation of the
 * job is provided by a separate class, which Job is instantiated on.
 * That class is meant to provide 4 static member functions:
 *
 * std::string name() - Returns the name of the job. There's no
 * restrictions on the name, but it must be globally unique across all
 * Jobs in the executable.
 *
 * void init(<inputs>) - Called once per worker invocation. Used to
 * set up global state before run() is called.
 *
 * <outputs> run(<inputs>) - Run the job. This may be called multiple
 * times per work invocation (with different inputs). init() will only
 * be called once beforehand.
 *
 * void fini() - Called once when work is cleaning up. Meant to tear
 * down any global state.
 *
 * The inputs of init() and the inputs/outputs of run() can be any set
 * of types which are blob serializable/deserializable, in addition to
 * the special marker types (described below). run() normally returns
 * one type. If you want to return multiple types, use Multi<>.
 *
 * To create a Job, instantiate it with the appropriate class and
 * declare a static instance of it. The Job *must* have static
 * lifetime.
 */
template <typename C>
struct Job : public detail::JobBase {
  Job();

  // The inferred Ref types from the declared inputs/outputs of C's
  // member functions.
  using ConfigT = typename detail::ConfigRefs<C>::type;
  using InputsT = typename detail::InputRefs<C>::type;
  using ReturnT = typename detail::ReturnRefs<C>::type;

private:
  void init(const folly::fs::path&) const override;
  void fini() const override;
  void run(const folly::fs::path&, const folly::fs::path&) const override;
};

//////////////////////////////////////////////////////////////////////

// "Marker" types. These are used to control how types are mapped to
// Refs. Useful, for example, of returning a vector of Refs instead of
// a Ref of a vector. Note that marker types never appear within a
// Ref.

// By default: T -> Ref<T>

template <typename T>
struct Variadic {
  // Variadic<T> -> std::vector<Ref<T>>
  using Type = T;
  std::vector<T> vals;
};

template <typename T>
struct Opt {
  // Opt<T> -> Optional<Ref<T>>
  using Type = T;
  Optional<T> val;
};

// Multi is only valid as a return type.
template <typename... Ts>
struct Multi {
  // Multi<T1, T2, ...> -> std::tuple<Ref<T1>, Ref<T2>, ....>
  /* implicit */ template <typename... Us> Multi(std::tuple<Us...> t)
    : vals{std::move(t)} {}
  std::tuple<Ts...> vals;
};

//////////////////////////////////////////////////////////////////////

// Identifier for a Ref. Used by the implementation to track them. The
// meaning of the identifier is private to the implementation.
struct RefId {
  RefId(std::string, size_t);

  std::string toString() const;
  bool operator==(const RefId&) const;
  bool operator!=(const RefId&) const;

  // Despite their names, these fields can be used for anything.
  std::string m_id;
  size_t m_size;
};

// Represents a piece of data "inside" the extern-worker
// framework. The data may not even exist locally (it could be on disk
// or in the network). A Ref is basically a RefId and the type of the
// data. Only Client can create Refs, so "type-punning" is
// impossible. A Ref is only usable with the Client that produced it
// and cannot outlive the Client.
template <typename T>
struct Ref {
  const RefId& id() const { return m_id; }
  // Whether this ref came from a "fallback" operation (see below with
  // Client). This is exposed mainly for testing. Users shouldn't
  // care.
  bool fromFallback() const { return m_fromFallback; }
private:
  Ref(RefId, bool);
  RefId m_id;
  bool m_fromFallback;
  friend struct Client;
};

//////////////////////////////////////////////////////////////////////

// This is meant for internal usage and is here (and not in detail) so
// implementations outside of these files can use it. It represents a
// particular operation and has some stuff for tracking time and
// TRACE.
struct RequestId {
  explicit RequestId(const char* type);
  ~RequestId();

  RequestId(const RequestId&) = delete;
  RequestId(RequestId&&) = default;
  RequestId& operator=(const RequestId&) = delete;
  RequestId& operator=(RequestId&&) = default;

  std::string tracePrefix() const;
  std::string toString() const;

private:
  uint64_t m_id;
  const char* m_type;
  Optional<detail::Timer> m_timer;

  static std::atomic<uint64_t> s_next;
  static std::atomic<uint64_t> s_active;

  TRACE_SET_MOD(extern_worker);
};

//////////////////////////////////////////////////////////////////////

// More stuff for the Client/Client::Impl interface here out of
// convenience.
using IdVec = std::vector<RefId>;
// A "blob" is a string containing some arbitrary binary data
using BlobVec = std::vector<std::string>;
using PathVec = std::vector<folly::fs::path>;

// These are used to describe inputs in a generic way to
// Client::Impl. An input can be a RefId, an optional RefId, or a
// vector of RefIds.
using RefVal = boost::variant<RefId, Optional<RefId>, IdVec>;
using RefValVec = std::vector<RefVal>;

// Likewise, these describe outputs to Client::Impl. We only need to
// represent the type since there's no id beforehand.
enum class OutputType { Val, Opt, Vec };

//////////////////////////////////////////////////////////////////////

// Configeration controlling the behavior of Client.
struct Options {
  // Whether to use the always available "subprocess"
  // implementation. This uses fork+exec (and stores data on disk).
  enum class UseSubprocess {
    Always, // Always use subprocess
    Fallback, // Attempt to use another backend, but if not available,
              // use subprocess.
    Never // Never use subprocess. Throw error if nothing else is
          // available.
  };
  Options& setUseSubprocess(UseSubprocess u) {
    m_useSubprocess = u;
    return *this;
  }

  // The implementation may need to store data on disk (subprocess for
  // example). Location where to store such things.
  Options& setWorkingDir(folly::fs::path dir) {
    m_workingDir = std::move(dir);
    return *this;
  }

  // Time out on job execution. Best effort, implementations may not
  // support it (subprocess does not).
  Options& setTimeout(std::chrono::seconds s) {
    m_timeout = s;
    return *this;
  }

  // Whether to cache execution of jobs. Not all implementations cache
  // execution (subprocess does not), so is a noop on those.
  Options& setCacheExecs(bool c) {
    m_cacheExecs = c;
    return *this;
  }

  // The minimum TTL before a cache entry is refreshed. If the
  // implementation caches data, we'll consider the data as "not
  // present" if its TTL drops below this value. This allows us to
  // re-upload the value (and refresh its TTL) before it actually
  // expires.
  Options& setMinTTL(std::chrono::seconds s) {
    m_minTTL = s;
    return *this;
  }

  // Implementations which rely on hashing can use EdenFS to avoid
  // hashing the file. This controls that.
  Options& setUseEdenFS(bool u) {
    m_useEdenFS = u;
    return *this;
  }

  // Whether to cleanup data stored on disk when Client is
  // destroyed. This can take a very long time for lots of data (and
  // can hinder debugging), so can be disabled.
  Options& setCleanup(bool c) {
    m_cleanup = c;
    return *this;
  }

  // Some implementations have a notion of "use-case". This provides
  // one which can control whether those implementations are enabled.
  Options& setUseCase(std::string u) {
    m_useCase = std::move(u);
    return *this;
  }

  UseSubprocess m_useSubprocess{UseSubprocess::Fallback};
  folly::fs::path m_workingDir{folly::fs::temp_directory_path()};
  std::chrono::seconds m_timeout{std::chrono::minutes{15}};
  std::chrono::seconds m_minTTL{std::chrono::hours{3}};
  bool m_cacheExecs{true};
  bool m_useEdenFS{true};
  bool m_cleanup{true};
  std::string m_useCase;
};

//////////////////////////////////////////////////////////////////////

// Encapsulates an instance of the extern-worker framework. This is
// responsible for producing/consuming Refs, and executing jobs with
// those Refs as inputs. The actual behavior of the Client (IE, where
// it stores the data, and where the workers run), depends on the
// specific Client::Impl in use. An implementation which uses
// fork+exec (and stores data on disk) is always available and will be
// used if nothing else is available (if so requested in Options). The
// fork+exec implementation can also serve as a "fallback"
// implementation if the main implementation throws an error. The
// assumption is that the fork+exec implementation is more reliable
// than anything else. For the most part, this is invisible to the
// user (though it may cause performance degradation).

struct Client {
  // Create a new Client with the given set of Options, and an
  // Executor. The executor will be used for any coro things if the
  // implementation requires it.
  explicit Client(folly::Executor::KeepAlive<>, const Options& = {});
  ~Client();

  // Return a descriptive string of the implementation currently in
  // use. Mainly for logging.
  const std::string& implName() const;
  // Return true if the implementation in use is the built-in
  // fork+exec implementation.
  bool usingSubprocess() const;

  // Loading. These take various different permutations of Refs, load
  // them, deserialize the blobs into the appropriate types, and
  // return the data in a matching format. Using the variations which
  // take multiple at once is more efficient than using multiple
  // calls.
  template <typename T> coro::Task<T> load(Ref<T>);

  template <typename T, typename... Ts>
  coro::Task<std::tuple<T, Ts...>> load(Ref<T>, Ref<Ts>...);

  template <typename T> coro::Task<std::vector<T>> load(std::vector<Ref<T>>);

  template <typename T, typename... Ts>
  coro::Task<std::vector<std::tuple<T, Ts...>>>
  load(std::vector<std::tuple<Ref<T>, Ref<Ts>...>>);

  // Storing files. These take either a path, or a vector of paths,
  // and upload the contents. This is semantically equivalent to
  // reading the file yourself and uploading the data as
  // blobs. However, it might be more efficient as some
  // implementations can deal with on-disk files specially. Note that
  // the returned Refs are for strings, since you're uploading the
  // contents of the file. If provided, "read" and/or "uploaded" will
  // be set to true if the file was read and whether it was actually
  // uploaded. If the file has been cached, neither may need to be
  // done. For the vector variant, the out params will be set to the
  // number of files actually read or uploaded.
  coro::Task<Ref<std::string>> storeFile(folly::fs::path,
                                         bool* read = nullptr,
                                         bool* uploaded = nullptr);

  coro::Task<std::vector<Ref<std::string>>>
  storeFile(std::vector<folly::fs::path>,
            size_t* read = nullptr,
            size_t* uploaded = nullptr);

  // Storing blobs. These take various different permutations of data,
  // serialize them (using BlobEncoder), store however the
  // implementation does, and return the appropriate Refs for
  // them. These have different names to avoid ambiguities (do you
  // want to upload a single vector of T, or multiple Ts passed as
  // vector?).
  template <typename T> coro::Task<Ref<T>> store(T);

  template <typename T, typename... Ts>
  coro::Task<std::tuple<Ref<T>, Ref<Ts>...>> store(T, Ts...);

  template <typename T>
  coro::Task<std::vector<Ref<T>>> storeMulti(std::vector<T>);

  template <typename T, typename... Ts>
  coro::Task<std::vector<std::tuple<Ref<T>, Ref<Ts>...>>>
  storeMultiTuple(std::vector<std::tuple<T, Ts...>>);

  // Execute a job with the given sets of inputs (and any config setup
  // params). The output of those job executions will be returned as a
  // vector of Refs. The exact format of the inputs and outputs is
  // determined (at compile time) by the job being run and matches the
  // job's specification. If "cached" is provided, it will be set to
  // true if the outputs are coming from a cached job and the job
  // isn't actually run.
  template <typename C> coro::Task<std::vector<typename Job<C>::ReturnT>>
  exec(const Job<C>& job,
       typename Job<C>::ConfigT config,
       std::vector<typename Job<C>::InputsT> inputs,
       bool* cached = nullptr);

  // Synthetically force a fallback event when storing data or
  // executing a job, as if the implementation failed. This is for
  // tests to force the fallback path to be exercised. You don't need
  // this otherwise.
  void forceFallback()   { m_forceFallback = true; }
  void unforceFallback() { m_forceFallback = false; }

  struct Impl;

private:
  std::unique_ptr<Impl> m_impl;
  LockFreeLazy<std::unique_ptr<Impl>> m_fallbackImpl;
  Options m_options;
  bool m_forceFallback;

  template <typename T, typename F> coro::Task<T> tryWithFallback(F, bool&);

  template <typename T> static T unblobify(std::string&&);
  template <typename T> static std::string blobify(T&&);

  static const std::array<OutputType, 1> s_valOutputType;
  static const std::array<OutputType, 1> s_vecOutputType;
  static const std::array<OutputType, 1> s_optOutputType;

  std::unique_ptr<Impl> makeFallbackImpl() const;

  TRACE_SET_MOD(extern_worker);
};

//////////////////////////////////////////////////////////////////////

// Actual implementation for Client. Implementations of Client::Impl
// control how data is stored and where the work is actually
// executed. Client always provides a "subprocess" implementation
// which uses the disk and fork+exec.
struct Client::Impl {
  virtual ~Impl() = default;

  // Name of the implementation. Mainly for logging.
  const std::string& name() const { return m_name; }

  // Whether this is a the special subprocess impl. Its treated
  // specially when it comes to falling back.
  virtual bool isSubprocess() const = 0;
  // An implementation can declare itself "disabled" at any point (for
  // example, due to some internal error). After that point, either
  // Client will fail, or the fallback subprocess implementation will
  // be used instead (depending on config).
  virtual bool isDisabled() const = 0;

  // Load some number of RefIds, returning them as blobs (in the same
  // order as requested).
  virtual coro::Task<BlobVec> load(const RequestId& requestId,
                                   IdVec ids) = 0;
  // Store some number of files and/or blobs, returning their
  // associated RefIds (in the same order as requested, with files
  // before blobs). "read" and "uploaded", if provided, are set to the
  // number of files actually read, and the number of items actually
  // uploaded to some storage. Some implementations can avoid reading
  // files directly, and others can use caching to avoid having to
  // upload already present data.
  virtual coro::Task<IdVec> store(const RequestId& requestId,
                                  PathVec files,
                                  BlobVec blobs,
                                  size_t* read,
                                  size_t* uploaded) = 0;

  // Execute a job with the given sets of inputs. The job will be
  // executed on a worker, with the job's run function called once for
  // each set of inputs. "cache", if provided, will be set to true if
  // the output comes from a cached result and the job didn't actually
  // run.
  virtual coro::Task<std::vector<RefValVec>>
  exec(const RequestId& requestId,
       const std::string& command,
       RefValVec config,
       std::vector<RefValVec> inputs,
       const folly::Range<const OutputType*>& output,
       bool* cached) = 0;
protected:
  explicit Impl(std::string name) : m_name{std::move(name)} {}
private:
  std::string m_name;
};

// Hook for providing an implementation. An implementation can set
// g_impl_hook to a function which optionally creates a Client::Impl.
using ImplHook =
  std::unique_ptr<Client::Impl>(*)(
    const Options&,
    folly::Executor::KeepAlive<>
  );
extern ImplHook g_impl_hook;

//////////////////////////////////////////////////////////////////////

// Maps a key to a Ref<V>, automatically storing the data if needed.
template <typename K, typename V>
struct RefCache {
  explicit RefCache(Client&);

  // Lookup the associated Ref for the given key. If there's no entry,
  // store the given value (using the Client provided in the ctor) and
  // return the Ref created.
  coro::Task<Ref<V>> get(const K&, const V&, folly::Executor::KeepAlive<>);
private:
  coro::AsyncMap<K, Ref<V>> m_map;
  Client& m_client;
};

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

#define incl_HPHP_EXTERN_WORKER_INL_H_
#include "hphp/util/extern-worker-inl.h"
#undef incl_HPHP_EXTERN_WORKER_INL_H_
