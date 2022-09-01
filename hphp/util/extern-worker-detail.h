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

#ifndef incl_HPHP_EXTERN_WORKER_DETAIL_H_
#error "extern-worker-detail.h should only be included by extern-worker.h"
#endif

#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"

#include <folly/String.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

/*
 * Implementation details for extern_worker that don't need to be
 * exposed to users.
 */

namespace HPHP::extern_worker {

//////////////////////////////////////////////////////////////////////

// This header gets included before any others, so these need to be
// forward declared.
template <typename T> struct Opt;
template <typename T> struct Variadic;
template <typename... Ts> struct Multi;

template <typename T> struct Ref;

extern int main(int, char**);

//////////////////////////////////////////////////////////////////////

namespace detail {

//////////////////////////////////////////////////////////////////////

// Scoped timing in debug builds. The emitted messages are supplied
// with lambdas to avoid overhead when the code is compiled out.
struct Timer {
  using Clock = std::chrono::steady_clock;

  // For when you're going to use stopWithMessage and don't want to
  // provide an initial message.
  Timer() : m_begin{Clock::now()}, m_msg{nullptr} {}

  explicit Timer(const char* msg) : m_begin{Clock::now()}, m_msg{msg} {}

  template <typename F>
  explicit Timer(const F& f) : m_begin{Clock::now()} {
    ONTRACE(2, [&] {
      m_str = f();
      m_msg = m_str.c_str();
    }());
  }

  Clock::duration elapsed() const { return Clock::now() - m_begin; }

  // Stop the timer early before destruction with the given message.
  template<typename F>
  void stopWithMessage(const F& f) {
    ONTRACE(2, [&] {
      m_msg = nullptr;
      FTRACE(2, "{} (took {})\n", f(), elapsedStr());
    }());
  }

  ~Timer() {
    ONTRACE(2, [this] {
      if (!m_msg) return;
      FTRACE(2, "{} took {}\n", m_msg, elapsedStr());
    }());
  }

private:
  std::string elapsedStr() const {
    namespace C = std::chrono;
    auto const d = C::duration_cast<C::duration<double>>(elapsed()).count();
    return folly::prettyPrint(d, folly::PRETTY_TIME, false);
  }

  Clock::time_point m_begin;
  const char* m_msg;
  std::string m_str;

  TRACE_SET_MOD(extern_worker);
};

// Execute the given lambda, wrapping it with a Timer.
template <typename F> auto time(const char* msg, const F& f) {
  Timer _{msg};
  return f();
}

template <typename F1, typename F2> auto time(const F1& f1, const F2& f2) {
  Timer _{f1};
  return f2();
}

//////////////////////////////////////////////////////////////////////

// Read/write to a file
extern std::string readFile(const std::filesystem::path&);
extern void writeFile(const std::filesystem::path&, const char*, size_t);

//////////////////////////////////////////////////////////////////////

// Matchers for the special "marker" classes

template <typename T>
struct IsVariadic : std::false_type {};
template <typename T>
struct IsVariadic<Variadic<T>> : std::true_type {};

template <typename T>
struct IsOpt : std::false_type {};
template <typename T>
struct IsOpt<Opt<T>> : std::true_type {};

template <typename... Ts>
struct IsMulti : std::false_type {};
template <typename... Ts>
struct IsMulti<Multi<Ts...>> : std::true_type {};

template <typename T>
using IsMarker = std::disjunction<IsVariadic<T>, IsOpt<T>, IsMulti<T>>;

//////////////////////////////////////////////////////////////////////

// Matchers for special non-marker classes

template <typename T>
struct IsRef : std::false_type {};
template <typename T>
struct IsRef<Ref<T>> : std::true_type {};

template <typename T>
struct IsVector : std::false_type {};
template <typename T>
struct IsVector<std::vector<T>> : std::true_type {};

template <typename T>
struct IsOptional : std::false_type {};
template <typename T>
struct IsOptional<Optional<T>> : std::true_type {};

template <typename T>
struct IsTuple : std::false_type {};
template <typename... Ts>
struct IsTuple<std::tuple<Ts...>> : std::true_type {};

//////////////////////////////////////////////////////////////////////

// Given a callable, Params<>::type is the callable's parameter types
// as a tuple.
template <typename> struct Params;
template <typename R, typename... P>
struct Params<R(P...)> {
  using type = std::tuple<
    typename std::remove_cv<typename std::remove_reference<P>::type>::type...
  >;
};

// Given a callable, Return<>::type is the callable's return type.
template <typename> struct Return;
template <typename R, typename... P>
struct Return<R(P...)> {
  using type =
    typename std::remove_cv<typename std::remove_reference<R>::type>::type;
};

//////////////////////////////////////////////////////////////////////

// Given a type T, ToRef<T>::type is equivalent Ref type. For most
// types this is just Ref<T>, but the variadic marker becomes a
// std::vector of Refs, and an optional marker becomes an Optional
// Ref.
template <typename T> struct ToRef {
  using type = Ref<T>;
};
template <typename T> struct ToRef<Variadic<T>> {
  using type = std::vector<typename ToRef<T>::type>;
};
template <typename T> struct ToRef<Opt<T>> {
  using type = Optional<typename ToRef<T>::type>;
};

//////////////////////////////////////////////////////////////////////

// Given a tuple of types, ToRefTuple<T>::type is a tuple with every
// type converted to its ToRef equivalent.
template <typename> struct ToRefTuple {};
template <typename... Ts> struct ToRefTuple<std::tuple<Ts...>> {
  using type = std::tuple<typename ToRef<Ts>::type...>;
};

// Same as ToRefTuple, except meant for return types. The only
// difference is that a return type of the Multi marker becomes a
// tuple.
template <typename T> struct ToRefReturn {
  using type = typename ToRef<T>::type;
};
template <typename... Ts> struct ToRefReturn<Multi<Ts...>> {
  using type = std::tuple<typename ToRef<Ts>::type...>;
};

//////////////////////////////////////////////////////////////////////

// Given a class (presumed to have static functions called init and
// run), return their parameters or return values, transformed with
// ToRef.
template <typename C> struct ConfigRefs {
  using type =
    typename ToRefTuple<typename Params<decltype(C::init)>::type>::type;
};

template <typename C> struct InputRefs {
  using type =
    typename ToRefTuple<typename Params<decltype(C::run)>::type>::type;
};

template <typename C> struct ReturnRefs {
  using type =
    typename ToRefReturn<typename Return<decltype(C::run)>::type>::type;
};

template <typename C> struct FiniRefs {
  using type =
    typename ToRefReturn<typename Return<decltype(C::fini)>::type>::type;
};

// The return type of exec called with a job. This is either
// ReturnRefs, or a tuple of ReturnRefs and FiniRefs (if fini returns
// anything).
template <typename C> struct ExecRet {
  using type =
    std::conditional_t<
      std::is_void_v<typename Return<decltype(C::fini)>::type>,
      std::vector<typename ReturnRefs<C>::type>,
      std::tuple<
        std::vector<typename ReturnRefs<C>::type>,
        typename FiniRefs<C>::type
      >
    >;
};

//////////////////////////////////////////////////////////////////////

// Iterate over a tuple. Just a wrapper around folly::for_each, which
// doesn't work on empty tuples for some reason.
template <typename F, typename... Ts>
void for_each(std::tuple<Ts...>&& tuple, F&& f) {
  if constexpr (sizeof...(Ts) != 0) {
    folly::for_each(std::move(tuple), std::forward<F>(f));
  }
}
template <typename F, typename... Ts>
void for_each(std::tuple<Ts...>& tuple, F&& f) {
  if constexpr (sizeof...(Ts) != 0) {
    folly::for_each(tuple, std::forward<F>(f));
  }
}
template <typename F, typename... Ts>
void for_each(const std::tuple<Ts...>& tuple, F&& f) {
  if constexpr (sizeof...(Ts) != 0) {
    folly::for_each(tuple, std::forward<F>(f));
  }
}

//////////////////////////////////////////////////////////////////////

// Empty class wrapping a Type. Meant to allow passing the type from
// typesToValues as an argument.
template <typename T> struct Tag { using Type = T; };

//////////////////////////////////////////////////////////////////////

// Instantiated on a tuple, and given a callable, call the callable
// for each type in that tuple. The callable is called with the tuple
// index as the first parameter, and Tag<T> as the second (where T is
// the type at that tuple index).
template <typename Tuple, typename F, size_t... Is>
auto typesToValuesImpl(F&& f,
                       std::index_sequence<Is...>) {
  return std::make_tuple(f(Is, Tag<std::tuple_element_t<Is, Tuple>>{})...);
}

template <typename Tuple, typename F>
auto typesToValues(F&& f) {
  return typesToValuesImpl<Tuple>(
    std::forward<F>(f),
    std::make_index_sequence<std::tuple_size<Tuple>{}>{}
  );
}

//////////////////////////////////////////////////////////////////////

// Abstracts away how a worker should obtain it's inputs, and write
// it's outputs. These functions are tightly coupled with the logic in
// JobBase::serialize and JobBase::deserialize.
struct ISource {
  virtual ~ISource() = default;
  virtual std::string blob() = 0;
  virtual Optional<std::string> optBlob() = 0;
  virtual std::vector<std::string> variadic() = 0;
  virtual void initDone() = 0;
  virtual bool inputEnd() const = 0;
  virtual void nextInput() = 0;
  virtual void finish() = 0;
};

struct ISink {
  virtual ~ISink() = default;
  virtual void blob(const std::string&) = 0;
  virtual void optBlob(const Optional<std::string>&) = 0;
  virtual void variadic(const std::vector<std::string>&) = 0;
  virtual void nextOutput() = 0;
  virtual void startFini() = 0;
  virtual void finish() = 0;
};

//////////////////////////////////////////////////////////////////////

// Base class for Jobs. This provide a consistent interface to invoke
// through.
struct JobBase {
  const std::string& name() const { return m_name; }
protected:
  explicit JobBase(const std::string& name);
  virtual ~JobBase() = default;

  template <typename T> static T deserialize(ISource&);
  template <typename T> static void serialize(const T&, ISink&);

private:
  template <typename T> static T deserializeBlob(std::string);
  template <typename T> static std::string serializeBlob(const T&);

  virtual void init(ISource&) const = 0;
  virtual void fini(ISink&) const = 0;
  virtual void run(ISource&, ISink&) const = 0;

  std::string m_name;

  friend int HPHP::extern_worker::main(int, char**);
};

//////////////////////////////////////////////////////////////////////

// Thrown by some implementations if the backend is busy. Depending on
// configuration, we might retry the action automatically.
struct Throttle : public Error {
  using Error::Error;
};

//////////////////////////////////////////////////////////////////////

}}
