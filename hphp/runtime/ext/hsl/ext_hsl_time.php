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

namespace HH\Lib\_Private\Native {

/* These functions are marked NoDoc for docs.hhvm.com:
 * - they are not intenteded to be used directly, other than by the HSL
 * - they should be moved to HH\Lib\_Private so the API is unstable.

/**
 * Returns the Unix timestamp, in nanoseconds, marking the beginning of
 * this request. The return value of this function will remain the same
 * throughout the request.
 *
 * @return int - The timestamp.
 */
<<__Native>>
function request_time_ns(): int;

}
