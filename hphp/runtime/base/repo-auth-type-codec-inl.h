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
#ifndef incl_HPHP_REPO_AUTH_TYPE_CODEC_INL_H_
#define incl_HPHP_REPO_AUTH_TYPE_CODEC_INL_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * We set the high bit in the tag if there's going to be a specialized
 * array id.  This should only be found when the tag is an array type.
 */
constexpr uint8_t kRATArrayDataBit = 0x80;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
size_t encodedRATSize(const unsigned char* pc) {
  using T = RepoAuthType::Tag;
  auto const rawTag = *pc;
  bool const highBitSet = rawTag & kRATArrayDataBit;
  auto const tag = static_cast<T>(rawTag & ~kRATArrayDataBit);
  switch (tag) {
  case T::Uninit:
  case T::InitNull:
  case T::Null:
  case T::Int:
  case T::OptInt:
  case T::Dbl:
  case T::OptDbl:
  case T::Res:
  case T::OptRes:
  case T::Bool:
  case T::OptBool:
  case T::SStr:
  case T::OptSStr:
  case T::Str:
  case T::OptStr:
  case T::Obj:
  case T::OptObj:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
    assert(!highBitSet);
    return 1;
  case T::SArr:
  case T::OptSArr:
  case T::Arr:
  case T::OptArr:
    return highBitSet ? 5 : 1;
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
    assert(!highBitSet);
    return 5;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

#endif
