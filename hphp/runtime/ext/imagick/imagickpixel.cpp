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
#include "hphp/runtime/base/array-init.h"

#include <utility>

namespace HPHP {

#define IMAGICKPIXEL_THROW imagickThrow<ImagickPixelException>

using PixelGetFunction = double (*)(const PixelWand*);
using PixelSetFunction = void (*)(PixelWand*, const double);

Object createImagickPixel(PixelWand* wand, bool owner) {
  Object ret = ImagickPixel::allocObject();
  setWandResource(s_ImagickPixel, ret, wand, owner);
  return ret;
}

Array createImagickPixelArray(size_t num, PixelWand* wands[], bool owner) {
  if (wands == nullptr) {
    return Array();
  } else {
    PackedArrayInit ret(num);
    for (int i = 0; i < num; ++i) {
      ret.appendWithRef(createImagickPixel(wands[i], owner));
    }
    return ret.toArray();
  }
}

ALWAYS_INLINE
req::ptr<WandResource<PixelWand>> getPixelWand(const Variant& obj) {
  if (!obj.isObject()) {
    IMAGICKPIXEL_THROW("Invalid color parameter provided");
  } else if (!obj.getObjectData()->instanceof(s_ImagickPixel)) {
    IMAGICKPIXEL_THROW(
      "The parameter must be an instance of ImagickPixel or a string");
  } else {
    auto wand = getPixelWandResource(obj.toCObjRef());
    return req::make<WandResource<PixelWand>>(wand->getWand(), false);
  }
}

req::ptr<WandResource<PixelWand>> newPixelWand() {
  auto ret = req::make<WandResource<PixelWand>>(NewPixelWand());
  if (ret->getWand() == nullptr) {
    IMAGICKPIXEL_THROW("Failed to allocate PixelWand structure");
  }
  return ret;
}

req::ptr<WandResource<PixelWand>> buildColorWand(const Variant& color) {
  if (!color.isString()) {
    return getPixelWand(color);
  }
  auto ret = newPixelWand();
  auto status = PixelSetColor(ret->getWand(), color.toCStrRef().c_str());
  if (status == MagickFalse) {
    IMAGICKPIXEL_THROW("Unrecognized color string");
  }
  return ret;
}

req::ptr<WandResource<PixelWand>> buildOpacityWand(const Variant& opacity) {
  if (!opacity.isInteger() && !opacity.isDouble()) {
    return getPixelWand(opacity);
  }
  auto ret = newPixelWand();
  PixelSetOpacity(ret->getWand(), opacity.toDouble());
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// class ImagickPixel
static bool HHVM_METHOD(ImagickPixel, clear) {
  auto wand = getPixelWandResource(this_);
  ClearPixelWand(wand->getWand());
  return true;
}

static void HHVM_METHOD(ImagickPixel, __construct, const String& color) {
  auto wand = NewPixelWand();
  if (wand == nullptr) {
    IMAGICKPIXEL_THROW("Failed to allocate PixelWand structure");
  } else {
    setWandResource(s_ImagickPixel, this_, wand);
  }
  if (!color.isNull() && !color.empty()) {
    if (PixelSetColor(wand, color.c_str()) == MagickFalse) {
      IMAGICKPIXEL_THROW("Unable to construct ImagickPixel");
    }
  }
}

static bool HHVM_METHOD(ImagickPixel, destroy) {
  return HHVM_MN(ImagickPixel, clear)(this_);
}

static Array HHVM_METHOD(ImagickPixel, getColor, bool normalized) {
  static const PixelGetFunction pixelGet[4] = {
    PixelGetRed, PixelGetGreen, PixelGetBlue, PixelGetAlpha
  };
  static const StaticString key[4] = {
    s_r, s_g, s_b, s_a
  };
  auto wand = getPixelWandResource(this_);

  ArrayInit ret(4, ArrayInit::Map{});
  for (int i = 0; i < 4; ++i) {
    double color = pixelGet[i](wand->getWand());
    if (i < 3 && !normalized) {
      color *= 255;
      ret.set(key[i], (int64_t)(color > 0.0 ? color + 0.5 : color - 0.5));
    } else {
      ret.set(key[i], color);
    }
  }
  return ret.toArray();
}

static String HHVM_METHOD(ImagickPixel, getColorAsString) {
  auto wand = getPixelWandResource(this_);
  return convertMagickString(PixelGetColorAsString(wand->getWand()));
}

static int64_t HHVM_METHOD(ImagickPixel, getColorCount) {
  auto wand = getPixelWandResource(this_);
  return PixelGetColorCount(wand->getWand());
}

static double HHVM_METHOD(ImagickPixel, getColorValue, int64_t color) {
  static const PixelGetFunction pixelGet[] = {
    PixelGetBlack,
    PixelGetBlue,
    PixelGetCyan,
    PixelGetGreen,
    PixelGetRed,
    PixelGetYellow,
    PixelGetMagenta,
    PixelGetOpacity,
    PixelGetAlpha,
    PixelGetFuzz
  };

  if (0 <= color && color < sizeof(pixelGet) / sizeof(pixelGet[0])) {
    auto wand = getPixelWandResource(this_);
    return pixelGet[color](wand->getWand());
  } else {
    IMAGICKPIXEL_THROW("Unknown color type");
  }
}

static Array HHVM_METHOD(ImagickPixel, getHSL) {
  auto wand = getPixelWandResource(this_);
  double hue, saturation, luminosity;
  PixelGetHSL(wand->getWand(), &hue, &saturation, &luminosity);
  return make_map_array(
    s_hue, hue,
    s_saturation, saturation,
    s_luminosity, luminosity);
}

static bool isSimilar(ObjectData* this_, const Variant& color,
                      double fuzz, bool useQuantum) {
  auto wand = getPixelWandResource(this_);
  auto pixel = buildColorWand(color);
  if (useQuantum) {
    fuzz *= QuantumRange;
  }
  return IsPixelWandSimilar(wand->getWand(), pixel->getWand(), fuzz)
    != MagickFalse;
}

static bool HHVM_METHOD(ImagickPixel, isPixelSimilar,
    const Variant& color, double fuzz) {
  return isSimilar(this_, color, fuzz, true);
}

static bool HHVM_METHOD(ImagickPixel, isSimilar,
    const Variant& color, double fuzz) {
  raiseDeprecated(s_ImagickPixel.c_str(), "isSimilar",
                  s_ImagickPixel.c_str(), "isPixelSimilar");
  return isSimilar(this_, color, fuzz, false);
}

static bool HHVM_METHOD(ImagickPixel, setColor,
    const String& color) {
  auto wand = getPixelWandResource(this_);
  if (PixelSetColor(wand->getWand(), color.c_str()) == MagickFalse) {
    IMAGICKPIXEL_THROW("Unable to set ImagickPixel color");
  }
  return true;
}

static bool HHVM_METHOD(ImagickPixel, setColorValue,
    int64_t color, double value) {
  static const PixelSetFunction pixelSet[] = {
    PixelSetBlack,
    PixelSetBlue,
    PixelSetCyan,
    PixelSetGreen,
    PixelSetRed,
    PixelSetYellow,
    PixelSetMagenta,
    PixelSetOpacity,
    PixelSetAlpha,
    PixelSetFuzz
  };

  if (0 <= color && color < sizeof(pixelSet) / sizeof(pixelSet[0])) {
    auto wand = getPixelWandResource(this_);
    pixelSet[color](wand->getWand(), value);
    return true;
  } else {
    IMAGICKPIXEL_THROW("Unknown color type");
  }
}

static bool HHVM_METHOD(ImagickPixel, setHSL,
    double hue, double saturation, double luminosity) {
  auto wand = getPixelWandResource(this_);
  PixelSetHSL(wand->getWand(), hue, saturation, luminosity);
  return true;
}

#undef IMAGICKPIXEL_THROW

void loadImagickPixelClass() {
  HHVM_ME(ImagickPixel, clear);
  HHVM_ME(ImagickPixel, __construct);
  HHVM_ME(ImagickPixel, destroy);
  HHVM_ME(ImagickPixel, getColor);
  HHVM_ME(ImagickPixel, getColorAsString);
  HHVM_ME(ImagickPixel, getColorCount);
  HHVM_ME(ImagickPixel, getColorValue);
  HHVM_ME(ImagickPixel, getHSL);
  HHVM_ME(ImagickPixel, isPixelSimilar);
  HHVM_ME(ImagickPixel, isSimilar);
  HHVM_ME(ImagickPixel, setColor);
  HHVM_ME(ImagickPixel, setColorValue);
  HHVM_ME(ImagickPixel, setHSL);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
