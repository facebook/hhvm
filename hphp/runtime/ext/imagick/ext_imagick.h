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

#include <wand/MagickWand.h>

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/imagick/constants.h"

namespace HPHP {

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
    static Object allocObject(CVarRef arg) { \
      Object ret = allocObject(); \
      TypedValue dummy; \
      g_vmContext->invokeFunc(&dummy, \
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
  explicit WandResource(Wand* wand, bool owner):
    m_wand(wand), m_owner(owner) {
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
WandResource<Wand>* getWandResource(const StaticString& className,
                                           CObjRef obj) {
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

template<typename Wand>
ALWAYS_INLINE
void setWandResource(const StaticString& className,
                            CObjRef obj,
                            Wand* wand,
                            bool destroy = true) {
  auto res = Resource(NEWOBJ(WandResource<Wand>(wand, destroy)));
  obj->o_set("wand", res, className.get());
}

ALWAYS_INLINE
WandResource<MagickWand>* getMagickWandResource(CObjRef obj) {
  return getWandResource<MagickWand>(s_Imagick, obj);
}

ALWAYS_INLINE
WandResource<DrawingWand>* getDrawingWandResource(CObjRef obj) {
  return getWandResource<DrawingWand>(s_ImagickDraw, obj);
}

ALWAYS_INLINE
WandResource<PixelWand>* getPixelWandResource(CObjRef obj) {
  return getWandResource<PixelWand>(s_ImagickPixel, obj);
}

ALWAYS_INLINE
WandResource<PixelIterator>* getPixelIteratorResource(CObjRef obj) {
  auto ret = getWandResource<PixelIterator>(s_ImagickPixelIterator, obj);
  if (ret->getWand() == nullptr) {
    imagickThrow<ImagickPixelIteratorException>(
      "ImagickPixelIterator is not initialized correctly");
  } else {
    return ret;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Common Helper
ALWAYS_INLINE
void raiseDeprecated(const char* className, const char* methodName) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED,
                "%s::%s method is deprecated and it's use should be avoided",
                className, methodName);
}

ALWAYS_INLINE
void raiseDeprecated(const char* className,
                     const char* methodName,
                     const char* newClass,
                     const char* newMethod) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED,
                "%s::%s is deprecated. %s::%s should be used instead",
                className, methodName, newClass, newMethod);
}

template<typename T>
ALWAYS_INLINE
void freeMagickMemory(T* &resource) {
  if (resource != nullptr) {
    MagickRelinquishMemory(resource);
    resource = nullptr;
  }
}

ALWAYS_INLINE
String convertMagickString(char* &&s) {
  if (s == nullptr) {
    return null_string;
  } else {
    String ret(s);
    freeMagickMemory(s);
    return ret;
  }
}

//////////////////////////////////////////////////////////////////////////////
// ImagickPixel Helper
PixelWand* getPixelWand(CVarRef color, bool& allocated);

Array createImagickPixelArray(size_t num, PixelWand* wands[], bool destroy);

//////////////////////////////////////////////////////////////////////////////
// Init Module
void loadImagickClass();

void loadImagickDrawClass();

void loadImagickPixelClass();

void loadImagickPixelIteratorClass();

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_EXT_IMAGICK_H_
