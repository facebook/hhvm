<?hh

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

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
async function main(): Awaitable<void> {
  try {
    _OS\socketpair(_OS\AF_INET6, _OS\SOCK_STREAM, 0);
  } catch (_OS\ErrnoException $e) {
    \var_dump($e->getMessage());
  }
  list($a, $b) = _OS\socketpair(_OS\AF_UNIX, _OS\SOCK_STREAM, 0);
  \var_dump(vec[_OS\getpeername($a), _OS\getpeername($b)]);
  \var_dump(vec[_OS\getsockname($a), _OS\getsockname($b)]);
  _OS\write($a, "Foo\n");
  \var_dump(_OS\read($b, 1024));
  _OS\write($b, "Bar\n");
  \var_dump(_OS\read($a, 1024));
}
