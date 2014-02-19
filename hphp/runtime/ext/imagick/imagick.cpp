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

#define IMAGICK_THROW imagickThrow<ImagickException>

//////////////////////////////////////////////////////////////////////////////
// class Imagick
static void HHVM_METHOD(Imagick, __construct, CVarRef files) {
  auto wand = NewMagickWand();
  if (wand == nullptr) {
    IMAGICK_THROW("Failed to create ImagickDraw object");
  } else {
    setWandResource(s_Imagick, this_, wand);
  }
  // todo: handle uri, stream, virtual;
  // todo: handle array of files
  // todo: handle error
  if (files.isString()) {
    MagickReadImage(wand, files.asCStrRef().c_str());
  }
}

static bool HHVM_METHOD(Imagick, drawImage, CObjRef draw) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImage,
    const String& filename) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImageFile,
    CResRef filehandle) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImages,
    const String& filename, bool adjoin) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImagesFile,
    CResRef filehandle) {
  throw NotImplementedException("Not Implemented");
}

#undef IMAGICK_THROW

void loadImagickClass() {
  HHVM_ME(Imagick, __construct);
  HHVM_ME(Imagick, drawImage);
  HHVM_ME(Imagick, writeImage);
  HHVM_ME(Imagick, writeImageFile);
  HHVM_ME(Imagick, writeImages);
  HHVM_ME(Imagick, writeImagesFile);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
