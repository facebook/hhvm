<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {
  function server_warmup_status(): string;
  function execution_context()[read_globals]: string;

  <<__NoAutoDynamic>>
  function array_mark_legacy<T>(T $in, bool $recursive = false)[]: T;
  <<__NoAutoDynamic>>
  function array_unmark_legacy<T>(T $in, bool $recursive = false)[]: T;
  <<__NoAutoDynamic>>
  function array_mark_legacy_recursive<T>(T $in)[]: T;
  <<__NoAutoDynamic>>
  function array_unmark_legacy_recursive<T>(T $in)[]: T;
  function is_array_marked_legacy(mixed $in)[]: bool;
}
namespace {
  const float INF;
  const float NAN;
  <<__PHPStdLib>>
  function connection_aborted(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function connection_status(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function connection_timeout(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function constant(string $name): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function defined(
    string $name,
    bool $autoload = true,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function get_browser(
    HH\FIXME\MISSING_PARAM_TYPE $user_agent = null,
    HH\FIXME\MISSING_PARAM_TYPE $return_array = false,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function ignore_user_abort(
    bool $setting = false,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function pack(
    string $format,
    HH\FIXME\MISSING_PARAM_TYPE ...$args
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function sleep(int $seconds): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function usleep(int $micro_seconds): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function time_nanosleep(
    int $seconds,
    int $nanoseconds,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function time_sleep_until(float $timestamp): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function uniqid(
    string $prefix = "",
    bool $more_entropy = false,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function unpack(
    string $format,
    string $data,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function sys_getloadavg(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function hphp_process_abort(
    HH\FIXME\MISSING_PARAM_TYPE $magic,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function hphp_to_string(
    HH\FIXME\MISSING_PARAM_TYPE $v,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
}
