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

inline bool cellToBool(const Cell* cell) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:          return false;
  case KindOfInt64:         return cell->m_data.num != 0;
  case KindOfBoolean:       return cell->m_data.num;
  case KindOfDouble:        return cell->m_data.dbl != 0;
  case KindOfStaticString:
  case KindOfString:        return cell->m_data.pstr->toBoolean();
  case KindOfArray:         return !cell->m_data.parr->empty();
  case KindOfObject:        // TODO: should handle o_toBoolean?
                            return true;
  default:                  break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
