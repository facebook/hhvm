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

#ifndef incl_HPHP_EXT_IMAGICK_H_
#define incl_HPHP_EXT_IMAGICK_H_

#include <vector>

#include <wand/MagickWand.h>

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/imagick/constants.h"
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////
// ImagickExtension
class ImagickExtension : public Extension {
 public:
  ImagickExtension();
  virtual void moduleInit();
  virtual void threadInit();

  static bool hasLocaleFix();
  static bool hasProgressMonitor();

 private:
  struct ImagickIniSetting {
    bool m_locale_fix;
    bool m_progress_monitor;
  };

  static DECLARE_THREAD_LOCAL(ImagickIniSetting, s_ini_setting);
};

//////////////////////////////////////////////////////////////////////////////
// PHP Exceptions and Classes
#define IMAGICK_DEFINE_CLASS(CLS) \
  class CLS { \
   public: \
    static Object allocObject() { \
      if (cls == nullptr) { \
        initClass(); \
      } \
      return ObjectData::newInstance(cls); \
    } \
    \
    static Object allocObject(const Variant& arg) { \
      Object ret = allocObject(); \
      TypedValue dummy; \
      g_context->invokeFunc(&dummy, \
                              cls->getCtor(), \
                              make_packed_array(arg), \
                              ret.get()); \
      return ret; \
    } \
    \
   private: \
    static void initClass() { \
      cls = Unit::lookupClass(StringData::Make(#CLS)); \
    } \
    \
    static HPHP::Class* cls; \
  };

IMAGICK_DEFINE_CLASS(ImagickException)
IMAGICK_DEFINE_CLASS(ImagickDrawException)
IMAGICK_DEFINE_CLASS(ImagickPixelException)
IMAGICK_DEFINE_CLASS(ImagickPixelIteratorException)

IMAGICK_DEFINE_CLASS(Imagick)
IMAGICK_DEFINE_CLASS(ImagickDraw)
IMAGICK_DEFINE_CLASS(ImagickPixel)
IMAGICK_DEFINE_CLASS(ImagickPixelIterator)

#undef IMAGICK_DEFINE_CLASS

template<typename T>
void imagickThrow(const char* fmt, ...)
  ATTRIBUTE_PRINTF(1, 2) ATTRIBUTE_NORETURN;

template<typename T>
void imagickThrow(const char* fmt, ...) {
  va_list ap;
  std::string msg;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  throw T::allocObject(msg);
}

//////////////////////////////////////////////////////////////////////////////
// WandResource
template<typename Wand>
class WandResource : public SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(WandResource<Wand>);

 public:
  explicit WandResource(Wand* wand, bool owner) :
      m_wand(wand), m_owner(owner) {
  }

  WandResource(WandResource<Wand> &&res) :
      m_wand(res.m_wand), m_owner(res.m_owner) {
    res.releaseWand();
  }

  ~WandResource() {
    clear();
  }

  void clear() {
    if (m_wand != nullptr) {
      if (m_owner) {
        destoryWand();
      }
      m_wand = nullptr;
    }
  }

  void destoryWand();

  Wand* getWand() {
    return m_wand;
  }

  Wand* releaseWand() {
    m_owner = false;
    return m_wand;
  }

 private:
  Wand* m_wand;
  bool m_owner;
};

template<typename Wand>
ALWAYS_INLINE
void WandResource<Wand>::sweep() {
  clear();
}

template<>
ALWAYS_INLINE
void WandResource<MagickWand>::destoryWand() {
  DestroyMagickWand(m_wand);
}

template<>
ALWAYS_INLINE
void WandResource<DrawingWand>::destoryWand() {
  DestroyDrawingWand(m_wand);
}

template<>
ALWAYS_INLINE
void WandResource<PixelWand>::destoryWand() {
  DestroyPixelWand(m_wand);
}

template<>
ALWAYS_INLINE
void WandResource<PixelIterator>::destoryWand() {
  DestroyPixelIterator(m_wand);
}

template<typename Wand>
ALWAYS_INLINE
void setWandResource(const StaticString& className,
                     const Object& obj,
                     Wand* wand,
                     bool destroy = true) {
  auto res = Resource(NEWOBJ(WandResource<Wand>(wand, destroy)));
  obj->o_set("wand", res, className.get());
}

template<typename Wand>
ALWAYS_INLINE
WandResource<Wand>* getWandResource(const StaticString& className,
                                    const Object& obj) {
  if (!obj.instanceof(className)) {
    return nullptr;
  }
  auto var = obj->o_get("wand", true, className.get());
  if (var.isNull()) {
    return nullptr;
  } else {
    return var.asCResRef().getTyped<WandResource<Wand>>();
  }
}

template<typename Wand, typename T>
ALWAYS_INLINE
WandResource<Wand>* getWandResource(const StaticString& className,
                                    const Object& obj,
                                    const std::string& msg) {
  auto ret = getWandResource<Wand>(className, obj);
  if (ret == nullptr || ret->getWand() == nullptr) {
    throw T::allocObject(msg);
  } else {
    return ret;
  }
}

ALWAYS_INLINE
WandResource<MagickWand>* getMagickWandResource(const Object& obj) {
  return getWandResource<MagickWand, ImagickException>(
    s_Imagick, obj,
    "Can not process invalid Imagick object");
}

ALWAYS_INLINE
WandResource<DrawingWand>* getDrawingWandResource(const Object& obj) {
  return getWandResource<DrawingWand, ImagickDrawException>(
    s_ImagickDraw, obj,
    "Can not process invalid ImagickDraw object");
}

ALWAYS_INLINE
WandResource<PixelWand>* getPixelWandResource(const Object& obj) {
  auto ret = getWandResource<PixelWand>(s_ImagickPixel, obj);
  assert(ret != nullptr && ret->getWand() != nullptr);
  return ret;
}

ALWAYS_INLINE
WandResource<PixelIterator>* getPixelIteratorResource(const Object& obj) {
  return getWandResource<PixelIterator, ImagickPixelIteratorException>(
    s_ImagickPixelIterator, obj,
    "ImagickPixelIterator is not initialized correctly");
}

//////////////////////////////////////////////////////////////////////////////
// IO
bool isMagickPseudoFormat(const String& path, char mode = '*');

using ImagickFileOp =
  std::function<MagickBooleanType(MagickWand*, const char*)>;
using ImagickHandleOp =
  std::function<MagickBooleanType(MagickWand*, FILE*)>;

void imagickReadOp(MagickWand* wand,
                   const String& path,
                   const ImagickFileOp& op);

void imagickWriteOp(MagickWand* wand,
                    const String& path,
                    const ImagickFileOp& op);

void imagickReadOp(MagickWand* wand,
                   const Resource& res,
                   const ImagickHandleOp& op);

void imagickWriteOp(MagickWand* wand,
                    const Resource& res,
                    const String& format,
                    const ImagickHandleOp& op);

//////////////////////////////////////////////////////////////////////////////
// Common Helper
void raiseDeprecated(const char* className, const char* methodName);

void raiseDeprecated(const char* className,
                     const char* methodName,
                     const char* newClass,
                     const char* newMethod);

template<typename T>
ALWAYS_INLINE
void freeMagickMemory(T* &resource) {
  if (resource != nullptr) {
    MagickRelinquishMemory(resource);
    resource = nullptr;
  }
}

String convertMagickString(char* &&str);

String convertMagickData(size_t size, unsigned char* &data);

template<typename T>
ALWAYS_INLINE
Array convertArray(size_t num, const T* arr) {
  PackedArrayInit ret(num);
  for (size_t i = 0; i < num; ++i) {
    ret.appendWithRef(arr[i]);
  }
  return ret.create();
}

template<typename T>
ALWAYS_INLINE
Array convertMagickArray(size_t num, T* &arr) {
  if (arr == nullptr) {
    return null_array;
  } else {
    Array ret = convertArray(num, arr);
    freeMagickMemory(arr);
    return ret;
  }
}

ALWAYS_INLINE
MagickBooleanType toMagickBool(bool value) {
  return value ? MagickTrue : MagickFalse;
}

MagickBooleanType withMagickLocaleFix(
    const std::function<MagickBooleanType()>& lambda);

std::vector<double> toDoubleArray(const Array& array);

std::vector<PointInfo> toPointInfoArray(const Array& coordinates);

//////////////////////////////////////////////////////////////////////////////
// Imagick Helper
Object createImagick(MagickWand* wand, bool owner);

Array magickQueryFonts(const char* pattern = "*");

Array magickQueryFormats(const char* pattern = "*");

String magickResolveFont(const String& fontName);

//////////////////////////////////////////////////////////////////////////////
// ImagickPixel Helper
Object createImagickPixel(PixelWand* wand, bool owner);

Array createImagickPixelArray(size_t num, PixelWand* wands[], bool owner);

WandResource<PixelWand> newPixelWand();

WandResource<PixelWand> buildColorWand(const Variant& color);

WandResource<PixelWand> buildOpacityWand(const Variant& opacity);

//////////////////////////////////////////////////////////////////////////////
// ImagickPixel Helper
Object createPixelIterator(const Object& magick);

Object createPixelRegionIterator(const Object& magick,
    int64_t x, int64_t y, int64_t columns, int64_t rows);

//////////////////////////////////////////////////////////////////////////////
// Init Module
void loadImagickClass();

void loadImagickDrawClass();

void loadImagickPixelClass();

void loadImagickPixelIteratorClass();

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_EXT_IMAGICK_H_
