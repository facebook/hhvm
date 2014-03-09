/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/imagick/ext_imagick.h"

namespace HPHP {

#define IMAGICKPIXELITERATOR_THROW imagickThrow<ImagickPixelIteratorException>

//////////////////////////////////////////////////////////////////////////////
// class ImagickPixelIterator
ALWAYS_INLINE
static void initPixelIterator(CObjRef this_, CObjRef magick) {
  auto wand = getMagickWandResource(magick);
  if (wand == nullptr || wand->getWand() == nullptr) {
    IMAGICKPIXELITERATOR_THROW("Invalid Imagick object passed");
  }

  auto it = NewPixelIterator(wand->getWand());
  if (it == nullptr) {
    IMAGICKPIXELITERATOR_THROW("Can not allocate ImagickPixelIterator");
  } else {
    setWandResource(s_ImagickPixelIterator, this_, it);
  }
}

ALWAYS_INLINE
static void initPixelRegioniIerator(CObjRef this_, CObjRef magick,
    int64_t x, int64_t y, int64_t columns, int64_t rows) {
  auto wand = getMagickWandResource(magick);
  if (wand == nullptr || wand->getWand() == nullptr) {
    IMAGICKPIXELITERATOR_THROW("Invalid Imagick object passed");
  }

  auto it = NewPixelRegionIterator(wand->getWand(), x, y, columns, rows);
  if (it == nullptr) {
    IMAGICKPIXELITERATOR_THROW("Can not allocate ImagickPixelIterator");
  } else {
    setWandResource(s_ImagickPixelIterator, this_, it);
  }
}

static Object HHVM_STATIC_METHOD(ImagickPixelIterator, getPixelIterator,
    CObjRef magick) {
  Object ret = ImagickPixelIterator::allocObject();
  initPixelIterator(ret, magick);
  return ret;
}

static Object HHVM_STATIC_METHOD(ImagickPixelIterator, getPixelRegionIterator,
    CObjRef magick, int64_t x, int64_t y, int64_t columns, int64_t rows) {
  Object ret = ImagickPixelIterator::allocObject();
  initPixelRegioniIerator(ret, magick, x, y, columns, rows);
  return ret;
}

static bool HHVM_METHOD(ImagickPixelIterator, clear) {
  auto it = getPixelIteratorResource(this_);
  ClearPixelIterator(it->getWand());
  return true;
}

static void HHVM_METHOD(ImagickPixelIterator, __construct, CObjRef magick) {
  initPixelIterator(this_, magick);
}

static bool HHVM_METHOD(ImagickPixelIterator, destroy) {
  return HHVM_MN(ImagickPixelIterator, clear)(this_);
}

static Array HHVM_METHOD(ImagickPixelIterator, getCurrentIteratorRow) {
  size_t num;
  auto it = getPixelIteratorResource(this_);
  auto wands = PixelGetCurrentIteratorRow(it->getWand(), &num);
  return createImagickPixelArray(num, wands, false);
}

static int64_t HHVM_METHOD(ImagickPixelIterator, getIteratorRow) {
  auto it = getPixelIteratorResource(this_);
  return PixelGetIteratorRow(it->getWand());
}

static Array HHVM_METHOD(ImagickPixelIterator, getNextIteratorRow) {
  size_t num;
  auto it = getPixelIteratorResource(this_);
  auto wands = PixelGetNextIteratorRow(it->getWand(), &num);
  return createImagickPixelArray(num, wands, false);
}

static Array HHVM_METHOD(ImagickPixelIterator, getPreviousIteratorRow) {
  size_t num;
  auto it = getPixelIteratorResource(this_);
  auto wands = PixelGetPreviousIteratorRow(it->getWand(), &num);
  return createImagickPixelArray(num, wands, false);
}

static bool HHVM_METHOD(ImagickPixelIterator, newPixelIterator,
    CObjRef magick) {
  raiseDeprecated(s_ImagickPixelIterator.c_str(), "newPixelIterator",
                  s_ImagickPixelIterator.c_str(), "getPixelIterator");
  initPixelIterator(this_, magick);
  return true;
}

static bool HHVM_METHOD(ImagickPixelIterator, newPixelRegionIterator,
    CObjRef magick, int64_t x, int64_t y, int64_t columns, int64_t rows) {
  raiseDeprecated(s_ImagickPixelIterator.c_str(), "newPixelRegionIterator",
                  s_ImagickPixelIterator.c_str(), "getPixelRegionIterator");
  initPixelRegioniIerator(this_, magick, x, y, columns, rows);
  return true;
}

static bool HHVM_METHOD(ImagickPixelIterator, resetIterator) {
  auto it = getPixelIteratorResource(this_);
  PixelResetIterator(it->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickPixelIterator, setIteratorFirstRow) {
  auto it = getPixelIteratorResource(this_);
  PixelSetFirstIteratorRow(it->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickPixelIterator, setIteratorLastRow) {
  auto it = getPixelIteratorResource(this_);
  PixelSetLastIteratorRow(it->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickPixelIterator, setIteratorRow, int64_t row) {
  auto it = getPixelIteratorResource(this_);
  if (PixelSetIteratorRow(it->getWand(), row) == MagickFalse) {
    IMAGICKPIXELITERATOR_THROW("Unable to set iterator row");
  } else {
    return true;
  }
}

static bool HHVM_METHOD(ImagickPixelIterator, syncIterator) {
  auto it = getPixelIteratorResource(this_);
  PixelSyncIterator(it->getWand());
  return true;
}

#undef IMAGICKPIXELITERATOR_THROW

void loadImagickPixelIteratorClass() {
  HHVM_STATIC_ME(ImagickPixelIterator, getPixelIterator);
  HHVM_STATIC_ME(ImagickPixelIterator, getPixelRegionIterator);
  HHVM_ME(ImagickPixelIterator, clear);
  HHVM_ME(ImagickPixelIterator, __construct);
  HHVM_ME(ImagickPixelIterator, destroy);
  HHVM_ME(ImagickPixelIterator, getCurrentIteratorRow);
  HHVM_ME(ImagickPixelIterator, getIteratorRow);
  HHVM_ME(ImagickPixelIterator, getNextIteratorRow);
  HHVM_ME(ImagickPixelIterator, getPreviousIteratorRow);
  HHVM_ME(ImagickPixelIterator, newPixelIterator);
  HHVM_ME(ImagickPixelIterator, newPixelRegionIterator);
  HHVM_ME(ImagickPixelIterator, resetIterator);
  HHVM_ME(ImagickPixelIterator, setIteratorFirstRow);
  HHVM_ME(ImagickPixelIterator, setIteratorLastRow);
  HHVM_ME(ImagickPixelIterator, setIteratorRow);
  HHVM_ME(ImagickPixelIterator, syncIterator);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
