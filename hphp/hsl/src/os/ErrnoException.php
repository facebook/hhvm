<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\C;
use namespace HH\Lib\_Private\_OS;


/**
 * Base class for exceptions reported via the C `errno` variable.
 *
 * Subclasses exist for some specific `Errno` values, such as:
 * - `ChildProcessException` (`ECHILD`)
 * - `ConnectionException` and its' subclasses, `BrokenPipeException`
 *   (`EPIPE`, `ESHUTDOWN`), `ConnectionAbortedException` (`ECONNABORTED`),
 *   `ConnectionRefusedException` (`ECONNREFUSED`), and
 *   `ConnectionResetException` (`ECONNRESET`)
 * - `AlreadyExistsException` (`EEXIST`)
 * - `NotFoundException` (`ENOENT`)
 * - `IsADirectoryException` (`EISDIR`)
 * - `IsNotADirectoryException` (`ENOTDIR`)
 * - `PermissionException` (`EACCESS`, `EPERM`)
 * - `ProcessLookupException` (`ESRCH`)
 * - `TimeoutError` (`ETIMEDOUT`)
 *
 * It is strongly recommended to catch subclasses instead of this class if a
 * suitable subclass is defined; for example:
 *
 * ```Hack
 * // ANTIPATTERN:
 * catch (OS\ErrnoException $e) {
 *   if ($e->getErrno() === OS\Errno::ENOENT) {
 *     do_stuff();
 *   }
 * }
 * // RECOMMENDED:
 * catch (OS\NotFoundException $_) {
 *   do_stuff();
 * }
 * ```
 *
 * If a suitable subclass is not defined, the antipattern is unavoidable.
 */
class ErrnoException extends \Exception {
  public function __construct(private Errno $errno, string $message) {
    parent::__construct($message);
    // Can't be in constructor: constructor takes int, but property - and
    // accessor - are mixed.
    $this->code = $errno;
  }

  final public function getErrno(): Errno{
    return $this->errno;
  }

  /** Deprecated for clarity, and potential future ambiguity.
   *
   * In the future, we may have exceptions with multiple 'codes', such as an
   * `errno` and a getaddrinfo `GAI` constant.
   *
   * Keeping logging rate at 0 so that generic code that works on any exception
   * stays happy.
   */
  <<__Deprecated("Use `getErrno()` instead", 0)>>
  final public function getCode()[]: Errno {
    return $this->errno;
  }
}

final class BlockingIOException extends ErrnoException {
  use _OS\ErrnoExceptionWithMultipleErrnosTrait;

  <<__Override>>
  public static function _getValidErrnos(): keyset<Errno> {
    return keyset[
      Errno::EAGAIN,
      Errno::EALREADY,
      Errno::EINPROGRESS,
    ];
  }
}

final class ChildProcessException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ECHILD;
  }
}

abstract class ConnectionException extends ErrnoException {
}

final class BrokenPipeException extends ConnectionException {
  use _OS\ErrnoExceptionWithMultipleErrnosTrait;

  <<__Override>>
  public static function _getValidErrnos(): keyset<Errno> {
    return keyset[Errno::EPIPE, Errno::ESHUTDOWN];
  }
}

final class ConnectionAbortedException extends ConnectionException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ECONNABORTED;
  }
}

final class ConnectionRefusedException extends ConnectionException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ECONNREFUSED;
  }
}

final class ConnectionResetException extends ConnectionException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ECONNRESET;
  }
}

final class AlreadyExistsException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::EEXIST;
  }
}

final class NotFoundException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ENOENT;
  }
}

final class IsADirectoryException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::EISDIR;
  }
}

final class IsNotADirectoryException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ENOTDIR;
  }
}

final class PermissionException extends ErrnoException {
  use _OS\ErrnoExceptionWithMultipleErrnosTrait;

  <<__Override>>
  public static function _getValidErrnos(): keyset<Errno> {
    return keyset[
      Errno::EACCES,
      Errno::EPERM,
    ];
  }
}

final class ProcessLookupException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ESRCH;
  }
}

final class TimeoutException extends ErrnoException {
  use _OS\ErrnoExceptionWithSingleErrnoTrait;

  <<__Override>>
  public static function _getValidErrno(): Errno {
    return Errno::ETIMEDOUT;
  }
}
