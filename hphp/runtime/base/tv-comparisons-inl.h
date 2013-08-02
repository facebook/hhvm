/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline bool cellEqual(Cell cell, int ival) {
  return cellEqual(cell, static_cast<int64_t>(ival));
}

inline bool cellLess(Cell cell, int ival) {
  return cellLess(cell, static_cast<int64_t>(ival));
}

inline bool cellGreater(Cell cell, int ival) {
  return cellGreater(cell, static_cast<int64_t>(ival));
}

//////////////////////////////////////////////////////////////////////

}
