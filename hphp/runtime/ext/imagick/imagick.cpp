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

#include "hphp/runtime/ext/ext_file.h"

namespace HPHP {

#define IMAGICK_THROW imagickThrow<ImagickException>

using MagickQueryFunction = char** (*)(const char*, size_t*);

ALWAYS_INLINE
static Array magickQuery(const char* pattern, MagickQueryFunction query) {
  size_t num = 0;
  auto result = query(pattern, &num);
  return convertMagickArray(num, result);
}

Array magickQueryFonts(const char* pattern /* = "*" */) {
  return magickQuery(pattern, MagickQueryFonts);
}

Array magickQueryFormats(const char* pattern /* = "*" */) {
  return magickQuery(pattern, MagickQueryFormats);
}

String magickResolveFont(const String& fontName) {
  Array fonts = magickQueryFonts();
  for (ArrayIter it(fonts); it; ++it) {
    if (strcasecmp(it.secondRefPlus().toCStrRef().c_str(),
                   fontName.c_str()) == 0) {
      return fontName;
    }
  }
  auto font = f_realpath(fontName);
  if (font.isBoolean() && !font.toBoolean()) {
    return null_string;
  } else {
    return font.toString();
  }
}

//////////////////////////////////////////////////////////////////////////////
// class Imagick
static bool HHVM_METHOD(Imagick, clear) {
  auto wand = getWandResource<MagickWand>(s_Imagick, this_);
  if (wand == nullptr || wand->getWand() == nullptr) {
    return false;
  } else {
    ClearMagickWand(wand->getWand());
    return true;
  }
}

static void HHVM_METHOD(Imagick, __construct, const Variant& files) {
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
    const String& file = files.toCStrRef();
    if (!file.empty()) {
      MagickReadImage(wand, file.c_str());
    }
  }
}

static bool HHVM_METHOD(Imagick, destroy) {
  return HHVM_MN(Imagick, clear)(this_);
}

static bool HHVM_METHOD(Imagick, drawImage, const Object& draw) {
  auto wand = getMagickWandResource(this_);
  auto drawing = getDrawingWandResource(draw);
  auto status = withMagickLocaleFix([&wand, &drawing]() {
    return MagickDrawImage(wand->getWand(), drawing->getWand());
  });
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to draw image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, newImage,
    int64_t cols, int64_t rows, const Variant& background, const String& format) {
  auto wand = getMagickWandResource(this_);
  WandResource<PixelWand> pixel(buildPixelWand(background));
  auto status = MagickNewImage(wand->getWand(), cols, rows, pixel.getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to create new image");
  }

  if (!format.empty()) {
    auto status = MagickSetImageFormat(wand->getWand(), format.c_str());
    if (status == MagickFalse) {
      IMAGICK_THROW("Unable to set the image format");
    }
  }

  return true;
}

static Array HHVM_METHOD(Imagick, queryFontMetrics,
    const Object& properties, const String& text, bool multiline) {
  throw NotImplementedException("Not Implemented");
}

static Array HHVM_METHOD(Imagick, queryFonts,
    const String& pattern) {
  return magickQueryFonts(pattern.c_str());
}

static Array HHVM_METHOD(Imagick, queryFormats,
    const String& pattern) {
  return magickQueryFormats(pattern.c_str());
}

static bool HHVM_METHOD(Imagick, writeImage,
    const String& filename) {
  auto wand = getMagickWandResource(this_);
  // todo: handle errors
  MagickWriteImage(wand->getWand(), filename.c_str());
  return true;
}

static bool HHVM_METHOD(Imagick, writeImageFile,
    const Resource& filehandle) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImages,
    const String& filename, bool adjoin) {
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(Imagick, writeImagesFile,
    const Resource& filehandle) {
  throw NotImplementedException("Not Implemented");
}

#undef IMAGICK_THROW

void loadImagickClass() {
  HHVM_ME(Imagick, clear);
  HHVM_ME(Imagick, __construct);
  HHVM_ME(Imagick, destroy);
  HHVM_ME(Imagick, drawImage);
  HHVM_ME(Imagick, newImage);
  HHVM_ME(Imagick, queryFontMetrics);
  HHVM_ME(Imagick, queryFonts);
  HHVM_ME(Imagick, queryFormats);
  HHVM_ME(Imagick, writeImage);
  HHVM_ME(Imagick, writeImageFile);
  HHVM_ME(Imagick, writeImages);
  HHVM_ME(Imagick, writeImagesFile);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
