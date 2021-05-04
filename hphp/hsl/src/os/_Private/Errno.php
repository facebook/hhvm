<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;

use namespace HH\Lib\{C, OS, Str};

<<__Memoize>>
function get_throw_errno_impl(): (function(OS\Errno, string): noreturn) {
  $single_code = keyset[
    OS\ChildProcessException::class,
    OS\ConnectionAbortedException::class,
    OS\ConnectionRefusedException::class,
    OS\ConnectionResetException::class,
    OS\AlreadyExistsException::class,
    OS\NotFoundException::class,
    OS\IsADirectoryException::class,
    OS\IsNotADirectoryException::class,
    OS\ProcessLookupException::class,
    OS\TimeoutException::class,
  ];
  $multiple_codes = keyset[
    OS\BlockingIOException::class,
    OS\BrokenPipeException::class,
    OS\PermissionException::class,
  ];

  $throws = new \HH\Lib\Ref(dict[]);
  $add_code = (OS\Errno $code, (function(string): noreturn) $impl) ==> {
    invariant(
      !C\contains_key($throws->value, $code),
      '%s has multiple exception implementations',
      $code,
    );
    $throws->value[$code] = $impl;
  };

  foreach ($single_code as $class) {
    $code = $class::_getValidErrno();
    $add_code($code, $msg ==> {
      throw new $class($msg);
    });
  }
  foreach ($multiple_codes as $class) {
    foreach ($class::_getValidErrnos() as $code) {
      $add_code($code, $msg ==> {
        throw new $class($code, $msg);
      });
    }
  }

  $throws = $throws->value;

  return ($code, $message) ==> {
    $override = $throws[$code] ?? null;
    if ($override) {
      $override($message);
    }
    throw new OS\ErrnoException($code, $message);
  };
}

function throw_errno(
  OS\Errno $errno,
  Str\SprintfFormatString $message,
  mixed ...$args
): noreturn {
  /* HH_FIXME[4027] needs literal format string */
  $message = Str\format($message, ...$args);
  invariant(
    $errno !== 0,
    "Asked to throw an errno ('%s'), but errno indicates success",
    $message,
  );
  $name = C\firstx(get_errno_names()[$errno]);
  $impl = get_throw_errno_impl();
  $impl($errno, Str\format("%s(%d): %s", $name, $errno, $message));
}
