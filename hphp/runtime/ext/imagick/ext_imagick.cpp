#include <wand/MagickWand.h>

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////
// Constant
static const StaticString
  s_r("r"),
  s_g("g"),
  s_b("b"),
  s_a("a"),
  s_hue("hue"),
  s_saturation("saturation"),
  s_luminosity("luminosity"),
  s_Imagick("Imagick"),
  s_ImagickDraw("ImagickDraw"),
  s_ImagickPixel("ImagickPixel"),
  s_ImagickPixelIterator("ImagickPixelIterator");

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
  }; \
  HPHP::Class* CLS::cls = nullptr;

IMAGICK_DEFINE_CLASS(ImagickException)
IMAGICK_DEFINE_CLASS(ImagickDrawException)
IMAGICK_DEFINE_CLASS(ImagickPixelException)
IMAGICK_DEFINE_CLASS(ImagickPixelIteratorException)

IMAGICK_DEFINE_CLASS(Imagick)
IMAGICK_DEFINE_CLASS(ImagickDraw)
IMAGICK_DEFINE_CLASS(ImagickPixel)
IMAGICK_DEFINE_CLASS(ImagickPixelIterator)

#undef IMAGICK_DEFINE_EXCEPTION

template<typename T>
static void imagickThrow(const char* fmt, ...)
  ATTRIBUTE_PRINTF(1, 2) ATTRIBUTE_NORETURN;

template<typename T>
static void imagickThrow(const char* fmt, ...) {
  va_list ap;
  std::string msg;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  throw T::allocObject(msg);
}

#define IMAGICK_THROW imagickThrow<ImagickException>
#define IMAGICKDRAW_THROW imagickThrow<ImagickDrawException>
#define IMAGICKPIXEL_THROW imagickThrow<ImagickPixelException>
#define IMAGICKPIXELITERATOR_THROW imagickThrow<ImagickPixelIteratorException>

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
static WandResource<Wand>* getWandResource(const StaticString& className,
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
static void setWandResource(const StaticString& className,
                            CObjRef obj,
                            Wand* wand,
                            bool destroy = true) {
  auto res = Resource(NEWOBJ(WandResource<Wand>(wand, destroy)));
  obj->o_set("wand", res, className.get());
}

ALWAYS_INLINE
static WandResource<MagickWand>* getMagickWandResource(CObjRef obj) {
  return getWandResource<MagickWand>(s_Imagick, obj);
}

ALWAYS_INLINE
static WandResource<PixelWand>* getPixelWandResource(CObjRef obj) {
  return getWandResource<PixelWand>(s_ImagickPixel, obj);
}

ALWAYS_INLINE
static WandResource<PixelIterator>* getPixelIteratorResource(CObjRef obj) {
  auto ret = getWandResource<PixelIterator>(s_ImagickPixelIterator, obj);
  if (ret->getWand() == nullptr) {
    IMAGICKPIXELITERATOR_THROW(
      "ImagickPixelIterator is not initialized correctly");
  } else {
    return ret;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Common Helper
ALWAYS_INLINE
static void raiseDeprecated(const char* className,
                            const char* methodName,
                            const char* newClass,
                            const char* newMethod) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED,
                "%s::%s is deprecated. %s::%s should be used instead",
                className, methodName, newClass, newMethod);
}

template<typename T>
ALWAYS_INLINE
static void freeMaigckMemory(T* &resource) {
  if (resource != nullptr) {
    MagickRelinquishMemory(resource);
    resource = nullptr;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Imagick Helper

//////////////////////////////////////////////////////////////////////////////
// ImagickPixel Helper
template<typename T>
static PixelWand* getPixelWand(CVarRef color, bool& allocated) {
  PixelWand* wand;
  allocated = false;

  if (color.isString()) {
    wand = NewPixelWand();
    if (wand == nullptr) {
      imagickThrow<T>("Failed to allocate PixelWand structure");
    }
    if (PixelSetColor(wand, color.asCStrRef().c_str()) == MagickFalse) {
      DestroyPixelWand(wand);
      imagickThrow<T>("Unrecognized color string");
    }
    allocated = true;
    return wand;
  } else if (color.isObject()) {
    if (color.instanceof(s_ImagickPixel)) {
      return getPixelWandResource(color.asCObjRef())->getWand();
    } else {
      imagickThrow<T>(
        "The parameter must be an instance of ImagickPixel or a string");
    }
  } else {
    imagickThrow<T>("Invalid color parameter provided");
  }
}

ALWAYS_INLINE
static Object createImagickPixel(PixelWand* wand, bool destroy) {
  Object ret = ImagickPixel::allocObject();
  setWandResource(s_ImagickPixel, ret, wand, destroy);
  return ret;
}

ALWAYS_INLINE
static Array createImagickPixelArray(size_t num,
                                     PixelWand* wands[],
                                     bool destroy) {
  if (wands == nullptr) {
    return null_array;
  } else {
    ArrayInit ret(num);
    for (int i = 0; i < num; ++i) {
      ret.set(i, createImagickPixel(wands[i], destroy));
    }
    return ret.create();
  }
}

//////////////////////////////////////////////////////////////////////////////
// ImagickPixelIterator Helper

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

//////////////////////////////////////////////////////////////////////////////
// class ImagickPixel
using PixelGetFunction = double (*)(const PixelWand*);
using PixelSetFunction = void (*)(PixelWand*, const double);

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

  ArrayInit ret(4);
  for (int i = 0; i < 4; ++i) {
    double color = pixelGet[i](wand->getWand());
    if (i < 3 && !normalized) {
      color *= 255;
      ret.set(key[i], (int64_t)(color > 0.0 ? color + 0.5 : color - 0.5));
    } else {
      ret.set(key[i], color);
    }
  }
  return ret.create();
}

static String HHVM_METHOD(ImagickPixel, getColorAsString) {
  auto wand = getPixelWandResource(this_);
  char* color_string = PixelGetColorAsString(wand->getWand());
  String ret(color_string);
  freeMaigckMemory(color_string);
  return ret;
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

  ArrayInit ret(3);
  ret.set(s_hue, hue);
  ret.set(s_saturation, saturation);
  ret.set(s_luminosity, luminosity);
  return ret.create();
}

static bool isSimilar(CObjRef this_, CVarRef color,
                      double fuzz, bool useQuantum) {
  auto wand = getPixelWandResource(this_);
  bool allocated = false;
  auto color_wand = getPixelWand<ImagickPixelException>(color, allocated);
  if (useQuantum) {
    fuzz *= QuantumRange;
  }
  auto status = IsPixelWandSimilar(wand->getWand(), color_wand, fuzz);
  if (allocated) {
    DestroyPixelWand(color_wand);
  }
  return status != MagickFalse;
}

static bool HHVM_METHOD(ImagickPixel, isPixelSimilar,
    CVarRef color, double fuzz) {
  return isSimilar(this_, color, fuzz, true);
}

static bool HHVM_METHOD(ImagickPixel, isSimilar,
    CVarRef color, double fuzz) {
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

//////////////////////////////////////////////////////////////////////////////

class ImagickExtension : public Extension {
 public:
  ImagickExtension() : Extension("imagick") {}
  virtual void moduleInit() {
    HHVM_ME(Imagick, __construct);

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

    loadSystemlib();
  }
} s_imagick_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(imagick);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
