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

#ifndef incl_HPHP_CLI_SERVER_EXT_H_
#define incl_HPHP_CLI_SERVER_EXT_H_

#include "hphp/util/trace.h"

#include <memory>
#include <string>

namespace HPHP {

struct ReturnedFdData {
  int fd;
};

struct LoanedFdData {
  int fd;
};

struct CLISuccess {};
struct CLIError {};

namespace detail {
template<class V>
typename std::enable_if<
  std::is_trivially_destructible<V>::value
>::type clear(V& val) {}

template<class V>
typename std::enable_if<
  !std::is_trivially_destructible<V>::value
>::type clear(V& val) { val.~V(); }

#define DECL(name)                                          \
  template<class... Args>                                   \
  name(CLISuccess, Args... args) : _succeeded(true) {       \
    new (&_result) TResult(std::forward<Args>(args)...);    \
  }                                                         \
  template<class... Args>                                   \
  name(CLIError, Args... args) : _succeeded(false) {        \
    new (&_error) TError(std::forward<Args>(args)...);      \
  }                                                         \
  union { TResult _result; TError _error; };                \
  bool _succeeded                                           \
/**/

template<class TResult, class TError>
struct TrivialDtor {
protected:
  DECL(TrivialDtor);
};

template<class TResult, class TError>
struct NonTrivialDtor {
  ~NonTrivialDtor() {
    if (_succeeded) clear(_result);
    else            clear(_error);
  }
protected:
  DECL(NonTrivialDtor);
};

template<class TResult, class TError>
using Base = typename std::conditional<
    std::is_trivially_destructible<TResult>::value &&
    std::is_trivially_destructible<TError>::value,
    TrivialDtor<TResult, TError>,
    NonTrivialDtor<TResult, TError>>::type;
} // namespace detail


template<class TResult, class TError>
struct CLISrvResult : detail::Base<TResult, TError> {
  using Base = detail::Base<TResult, TError>;
  template<class... Args>
  CLISrvResult(CLISuccess, Args... args) : Base(CLISuccess {}, std::forward<Args>(args)...) {}
  template<class... Args>
  CLISrvResult(CLIError, Args... args) : Base(CLIError {}, std::forward<Args>(args)...) {}

  CLISrvResult(const CLISrvResult&) = delete;
  CLISrvResult(CLISrvResult&&) = default;
  CLISrvResult& operator=(const CLISrvResult&) = delete;
  CLISrvResult& operator=(CLISrvResult&&) = default;

  bool succeeded() const { return this->_succeeded; }

  const TResult& result() const {
    assertx(succeeded());
    return this->_result;
  }

  const TError& error() const {
    assertx(!succeeded());
    return this->_error;
  }
};

} // namespace HPHP

#define INVOKE_ON_CLI_CLIENT(x, ...) \
  ::HPHP::detail::invoke_on_cli_client(::HPHP::detail::cli_get_extension_function_id(#x, x##_cli_client), x ## _cli_client, __VA_ARGS__)

#define CLI_CLIENT_HANDLER(x, ...) \
x##_cli_client(__VA_ARGS__)

#define CLI_REGISTER_HANDLER(name) \
  ::HPHP::detail::cli_register_handler(\
    ::HPHP::detail::cli_get_extension_function_id(#name, name##_cli_client),\
    [] (::HPHP::detail::CLIServerInterface& server) { HPHP::detail::cli_server_invoke_wrapped(server, name##_cli_client); } \
  );

#include "hphp/runtime/server/cli-server-ext-detail.h"

#endif
