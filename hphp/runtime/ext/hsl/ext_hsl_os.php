<?hh // strict

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
namespace HH\Lib\OS {

<<__NativeData("HSLFileDescriptor")>>
final class FileDescriptor {
  private function __construct() {}

  <<__Native>>
  public function __debugInfo(): darray;
}

}

namespace HH\Lib\_Private\_OS {


use type HH\Lib\OS\FileDescriptor;

final class ErrnoException extends \Exception {}

<<__Native>>
function open(string $path, int $flags, int $mode = 0): FileDescriptor;

/*
<<__Native>>
function pipe(): (FileDescriptor, FileDescriptor);
 */

<<__Native>>
function write(FileDescriptor $fd, string $data): int;

/*
<<__Native>>
function pwrite(FileDescriptor $fd, string $data, int $offset);
 */

<<__Native>>
function close(FileDescriptor $fd): void;

}
