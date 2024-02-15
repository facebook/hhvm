/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/native-prop-handler.h"

using std::pair;
using std::string;
using std::vector;

namespace HPHP {

#define IMAGICK_THROW imagickThrow<ImagickException>

// bool Imagick::$imagePending
static const StaticString s_imagePending("imagePending");

ALWAYS_INLINE
static bool getImagePending(const Object& imagick) {
  auto var = imagick->o_get(s_imagePending, true, s_Imagick);
  return var.toBoolean();
}

ALWAYS_INLINE
static void setImagePending(const Object& imagick, bool imagePending) {
  imagick->o_set(s_imagePending, imagePending, s_Imagick);
}

// class ImageGeometry
struct ImageGeometry {
  static const ImageGeometry empty_geometry;

  ImageGeometry(int64_t width, int64_t height) :
    m_width(width), m_height(height) {
  }

  explicit ImageGeometry(MagickWand* wand) :
    ImageGeometry(MagickGetImageWidth(wand), MagickGetImageHeight(wand)) {
  }

  bool empty() const {
    return m_width <= 0 || m_height <= 0;
  }

  bool operator==(const ImageGeometry& other) const {
    return m_width == other.m_width && m_height == other.m_height;
  }

  bool operator!=(const ImageGeometry& other) const {
    return m_width != other.m_width || m_height != other.m_height;
  }

  int64_t getWidth() const {
    return m_width;
  }

  int64_t getHeight() const {
    return m_height;
  }

  ImageGeometry scale(double ratio) const {
    static const double kEPS = 1e-8;
    return ImageGeometry(
      std::max<int64_t>(1, (int64_t)(m_width * ratio + kEPS)),
      std::max<int64_t>(1, (int64_t)(m_height * ratio + kEPS)));
  }

  ImageGeometry toThumbnail(ImageGeometry hint, bool bestfit) const {
    if (empty()) {
      return empty_geometry;
    }
    double ratio_w = (double)hint.m_width / (double)m_width;
    double ratio_h = (double)hint.m_height / (double)m_height;
    if (bestfit) {
      if (hint.empty()) {
        return empty_geometry;
      } else {
        return scale(std::min(ratio_w, ratio_h));
      }
    } else {
      if (!hint.empty()) {
        return hint;
      } else if (hint.m_width <= 0 && hint.m_height <= 0) {
        return empty_geometry;
      } else {
        return scale(std::max(ratio_w, ratio_h));
      }
    }
  }

  Array toArray() const {
    return make_dict_array(
      s_width, m_width,
      s_height, m_height);
  }

 private:
  int64_t m_width;
  int64_t m_height;
};

const ImageGeometry ImageGeometry::empty_geometry(0, 0);

// assertion
ALWAYS_INLINE
static void ensureImageHasFormat(MagickWand* wand) {
  String format = convertMagickString(MagickGetImageFormat(wand));
  if (format.empty()) {
    IMAGICK_THROW("Image has no format");
  }
}

ALWAYS_INLINE
static void ensurePageIsValid(
    int64_t x, int64_t y, int64_t width, int64_t height) {
  if (x < 0 || y < 0) {
    IMAGICK_THROW("The coordinates must be non-negative");
  }
  if (width <= 0 || height <= 0) {
    IMAGICK_THROW("The width and height must be greater than zero");
  }
}

ALWAYS_INLINE
static void ensureChannelMapIsValid(const String& map) {
  static const StaticString s_channels("RGBAOCYMKIP");
  for (int i = 0; i < map.size(); ++i) {
    if (s_channels.find(map[i]) == String::npos) {
      IMAGICK_THROW("The map contains disallowed characters");
    }
  }
}

// import/export ImagePixels
template<StorageType T> struct StorageTypeCPPType;
template<> struct StorageTypeCPPType<CharPixel> { using T = unsigned char; };
template<> struct StorageTypeCPPType<LongPixel> { using T = long; };
template<> struct StorageTypeCPPType<DoublePixel> { using T = double; };

ALWAYS_INLINE
static StorageType resolveStorageType(StorageType storage) {

#define substituteStorageType(from, to) \
  do { \
    if (storage == from) { \
      /* \
      raiseDeprecated(s_Imagick.c_str(), #from, \
                      s_Imagick.c_str(), #to); \
      */ \
      storage = to; \
    } \
  } while (false)

  substituteStorageType(FloatPixel, DoublePixel);
  substituteStorageType(ShortPixel, LongPixel);
  substituteStorageType(IntegerPixel, LongPixel);

#undef substituteStorageType

  if (storage != CharPixel &&
      storage != IntegerPixel &&
      storage != DoublePixel) {
    IMAGICK_THROW("Unknown storage format");
  } else {
    return storage;
  }
}

// misc
ALWAYS_INLINE
static String getImageMimeType(MagickWand* wand) {
  String format = convertMagickString(MagickGetImageFormat(wand));
  if (format.empty()) {
    return String();
  }
  String mimetype = convertMagickString(MagickToMime(format.c_str()));
  if (mimetype.empty()) {
    return String();
  }
  return mimetype;
}

//////////////////////////////////////////////////////////////////////////////
// Imagick Helper
using MagickQueryFunction = char** (*)(const char*, size_t*);

Object createImagick(MagickWand* wand) {
  Object ret = Imagick::allocObject();
  setWandResource(s_Imagick, ret, wand);
  return ret;
}

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
    if (strcasecmp(val(it.secondValPlus()).pstr->data(),
                   fontName.c_str()) == 0) {
      return fontName;
    }
  }
  auto font = HHVM_FN(realpath)(fontName);
  if (font.isBoolean() && !font.toBoolean()) {
    return String();
  } else {
    return font.toString();
  }
}

//////////////////////////////////////////////////////////////////////////////
// class Imagick
static bool HHVM_METHOD(Imagick, adaptiveBlurImage,
    double radius, double sigma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickAdaptiveBlurImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to adaptive blur image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, adaptiveResizeImage,
    int64_t columns, int64_t rows, bool bestfit) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand()).toThumbnail(
    {columns, rows}, bestfit);
  if (geometry.empty()) {
    IMAGICK_THROW("Invalid image geometry");
  }
  auto status = MagickAdaptiveResizeImage(
    wand->getWand(), geometry.getWidth(), geometry.getHeight());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to adaptive resize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, adaptiveSharpenImage,
    double radius, double sigma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickAdaptiveSharpenImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to adaptive sharpen image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, adaptiveThresholdImage,
    int64_t width, int64_t height, int64_t offset) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickAdaptiveThresholdImage(
    wand->getWand(), width, height, offset);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to adaptive threshold image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, addImage, const Object& source) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = getMagickWandResource(source);
  auto status = MagickAddImage(wand->getWand(), magick->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to add image");
  }
  MagickSetLastIterator(wand->getWand());
  return true;
}

static bool HHVM_METHOD(Imagick, addNoiseImage,
    int64_t noise_type, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickAddNoiseImageChannel(
    wand->getWand(), (ChannelType)channel, (NoiseType)noise_type);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to add image noise");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, affineTransformImage, const Object& matrix) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(matrix);
  auto status = MagickAffineTransformImage(
    wand->getWand(), drawing->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to affine transform image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, animateImages, const String& x_server) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSetFirstIterator(wand->getWand());
  auto status = MagickAnimateImages(wand->getWand(), x_server.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to animate images");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, annotateImage, const Object& draw_settings,
    double x, double y, double angle, const String& text) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(draw_settings);
  auto status = MagickAnnotateImage(
    wand->getWand(), drawing->getWand(), x, y, angle, text.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to annotate image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, appendImages, bool stack) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickAppendImages(wand->getWand(), toMagickBool(stack));
  if (magick == nullptr) {
    IMAGICK_THROW("Unable to append images");
  }
  return createImagick(magick);
}

static Object HHVM_METHOD(Imagick, averageImages) {
  raiseDeprecated(s_Imagick.c_str(), "averageImages");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto magick = MagickAverageImages(wand->getWand());
#pragma GCC diagnostic pop
  if (magick == nullptr) {
    IMAGICK_THROW("Averaging images failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, blackThresholdImage,
    const Variant& threshold) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(threshold);
  auto status = MagickBlackThresholdImage(wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to black threshold image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, blurImage,
    double radius, double sigma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickBlurImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to blur image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, borderImage,
    const Variant& bordercolor, int64_t width, int64_t height) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(bordercolor);
  auto status = MagickBorderImage(
    wand->getWand(), pixel->getWand(), width, height);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to border image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, charcoalImage,
    double radius, double sigma) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickCharcoalImage(wand->getWand(), radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to charcoal image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, chopImage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickChopImage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to chop image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, clear) {
  auto wand = getWandResource<MagickWand>(s_Imagick, Object{this_});
  if (!wand || wand->getWand() == nullptr) {
    return false;
  } else {
    ClearMagickWand(wand->getWand());
    setImagePending(Object{this_}, false);
    return true;
  }
}

static bool HHVM_METHOD(Imagick, clipImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickClipImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to clip image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, clipPathImage,
    const String& pathname, bool inside) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickClipImagePath(
    wand->getWand(), pathname.c_str(), toMagickBool(inside));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to clip path image");
  }
  return true;
}

static void HHVM_METHOD(Imagick, __clone) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = CloneMagickWand(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Cloning Imagick object failed");
  } else {
    setWandResource(s_Imagick, Object{this_}, magick);
  }
}

static bool HHVM_METHOD(Imagick, clutImage,
    const Object& lookup_table, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = getMagickWandResource(lookup_table);
  auto status = MagickClutImageChannel(
    wand->getWand(), (ChannelType)channel, magick->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW(
      "Unable to replace colors in the image from a color lookup table");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, coalesceImages) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickCoalesceImages(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Coalesce image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, colorFloodfillImage,
    const Variant& fill, double fuzz,
    const Variant& bordercolor, int64_t x, int64_t y) {
  raiseDeprecated(s_Imagick.c_str(), "colorFloodFillImage");

  auto wand = getMagickWandResource(Object{this_});
  auto fillPixel = buildColorWand(fill);
  auto borderPixel = buildColorWand(bordercolor);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickColorFloodfillImage(
    wand->getWand(), fillPixel->getWand(), fuzz, borderPixel->getWand(), x, y);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to color floodfill image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, colorizeImage,
    const Variant& colorize, const Variant& opacity) {
  auto wand = getMagickWandResource(Object{this_});
  auto colorPixel = buildColorWand(colorize);
  auto opacityPixel = buildOpacityWand(opacity);
  auto pixel = req::make<WandResource<PixelWand>>(
    ClonePixelWand(colorPixel->getWand()));

  if (pixel->getWand() == nullptr) {
    IMAGICK_THROW("Failed to allocate");
  } else {
    auto opacityValue = PixelGetOpacity(opacityPixel->getWand());
    auto alphaValue = PixelGetAlpha(opacityPixel->getWand());
    PixelSetOpacity(pixel->getWand(), opacityValue);
    PixelSetAlpha(pixel->getWand(), alphaValue);
  }

  auto status = MagickColorizeImage(
    wand->getWand(), pixel->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to colorize image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, combineImages, int64_t channelType) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickCombineImages(wand->getWand(), (ChannelType)channelType);
  if (magick == nullptr) {
    IMAGICK_THROW("Combine images failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, commentImage, const String& comment) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickCommentImage(wand->getWand(), comment.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to comment image");
  }
  return true;
}

static Array HHVM_METHOD(Imagick, compareImageChannels,
    const Object& image, int64_t channelType, int64_t metricType) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(image);
  double distortion;
  auto magick = MagickCompareImageChannels(
    wand->getWand(), wand2->getWand(),
    (ChannelType)channelType, (MetricType)metricType, &distortion);

  if (magick == nullptr) {
    IMAGICK_THROW("Compare image channels failed");
  } else {
    return make_vec_array(createImagick(magick), distortion);
  }
}

static Object HHVM_METHOD(Imagick, compareImageLayers, int64_t method) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickCompareImageLayers(
    wand->getWand(), (ImageLayerMethod)method);
  if (magick == nullptr) {
    IMAGICK_THROW("Compare image layers failed");
  }
  return createImagick(magick);
}

static Array HHVM_METHOD(Imagick, compareImages,
    const Object& compare, int64_t metric) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(compare);
  double distortion;
  auto magick = MagickCompareImages(
    wand->getWand(), wand2->getWand(),
    (MetricType)metric, &distortion);

  if (magick == nullptr) {
    IMAGICK_THROW("Compare images failed");
  } else {
    return make_vec_array(createImagick(magick), distortion);
  }
}

static bool HHVM_METHOD(Imagick, compositeImage,
    const Object& composite_object, int64_t composite,
    int64_t x, int64_t y, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(composite_object);
  MagickCompositeImageChannel(
    wand->getWand(), (ChannelType)channel,
    wand2->getWand(), (CompositeOperator)composite, x, y);
  return true;
}

static void HHVM_METHOD(Imagick, __construct, const Variant& files) {
  auto magick = NewMagickWand();
  if (magick == nullptr) {
    IMAGICK_THROW("Failed to create ImagickDraw object");
  } else {
    setWandResource(s_Imagick, Object{this_}, magick);
  }
  auto wand = getMagickWandResource(Object{this_});
  Array array = files.isString() ? make_vec_array(files)
              : files.isArray() ? files.toArray()
              : empty_vec_array();
  for (ArrayIter it(array); it; ++it) {
    imagickReadOp(
      wand->getWand(),
      tvCastToString(it.secondValPlus()),
      MagickReadImage
    );
  }
}

static bool HHVM_METHOD(Imagick, contrastImage, bool sharpen) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickContrastImage(wand->getWand(), toMagickBool(sharpen));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to contrast image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, contrastStretchImage,
    double black_point, double white_point, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickContrastStretchImageChannel(
    wand->getWand(), (ChannelType)channel, black_point, white_point);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to contrast strech image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, convolveImage,
    const Array& kernelArray, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto kernel = toDoubleArray(kernelArray);
  if (kernel.empty()) {
    IMAGICK_THROW("Unable to read matrix array");
  }
  auto order = (size_t)sqrt(kernel.size());
  auto status = MagickConvolveImageChannel(
    wand->getWand(), (ChannelType)channel, order, kernel.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to convolve image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, cropImage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickCropImage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to crop image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, cropThumbnailImage,
    int64_t width, int64_t height) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand());
  ImageGeometry thumbnail(width, height);
  MagickBooleanType status;

  if (thumbnail.empty()) {
    status = MagickFalse;
  } else if (thumbnail == geometry) {
    // Already at the size, just strip profiles
    status = MagickStripImage(wand->getWand());
  } else {
    auto ratio_w = (double)thumbnail.getWidth() / geometry.getWidth();
    auto ratio_h = (double)thumbnail.getHeight() / geometry.getHeight();
    auto thumbnail = geometry.scale(std::max(ratio_w, ratio_h));
    status = MagickThumbnailImage(
      wand->getWand(), thumbnail.getWidth(), thumbnail.getHeight());
    if (status != MagickFalse && thumbnail != ImageGeometry(width, height)) {
      auto crop_x = (thumbnail.getWidth() - width) / 2;
      auto crop_y = (thumbnail.getHeight() - height) / 2;
      status = MagickCropImage(
        wand->getWand(), width, height, crop_x, crop_y);
      if (status != MagickFalse) {
        status = MagickSetImagePage(wand->getWand(), width, height, 0, 0);
      }
    }
  }

  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to crop-thumbnail image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, cycleColormapImage, int64_t displace) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickCycleColormapImage(wand->getWand(), displace);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to cycle image colormap");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, decipherImage, const String& passphrase) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDecipherImage(wand->getWand(), passphrase.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to decipher image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, deconstructImages) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickDeconstructImages(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Deconstruct image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, deleteImageArtifact,
    const String& artifact) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDeleteImageArtifact(wand->getWand(), artifact.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to delete image artifact");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, deskewImage, double threshold) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDeskewImage(wand->getWand(), threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to deskew image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, despeckleImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDespeckleImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to despeckle image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, destroy) {
  return HHVM_MN(Imagick, clear)(this_);
}

static bool HHVM_METHOD(Imagick, displayImage, const String& servername) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDisplayImage(wand->getWand(), servername.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to display image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, displayImages, const String& servername) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickDisplayImages(wand->getWand(), servername.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to display images");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, distortImage,
    int64_t method, const Array& arguments, bool bestfit) {
  auto wand = getMagickWandResource(Object{this_});
  auto args = toDoubleArray(arguments);
  if (args.empty()) {
    IMAGICK_THROW("Can't read argument array");
  }
  auto status = MagickDistortImage(
    wand->getWand(), (DistortImageMethod)method,
    args.size(), args.data(), toMagickBool(bestfit));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to distort the image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, drawImage, const Object& draw) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(draw);
  auto status = withMagickLocaleFix([&wand, &drawing](){
    return MagickDrawImage(wand->getWand(), drawing->getWand());
  });
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to draw image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, edgeImage, double radius) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEdgeImage(wand->getWand(), radius);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to edge image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, embossImage, double radius, double sigma) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEmbossImage(wand->getWand(), radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to emboss image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, encipherImage, const String& passphrase) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEncipherImage(wand->getWand(), passphrase.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to encipher image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, enhanceImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEnhanceImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to enchance image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, equalizeImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEqualizeImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to equalize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, evaluateImage,
    int64_t op, double constant, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickEvaluateImageChannel(wand->getWand(),
    (ChannelType)channel, (MagickEvaluateOperator)op, constant);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to evaluate image");
  }
  return true;
}

template<StorageType T>
ALWAYS_INLINE
static vector<typename StorageTypeCPPType<T>::T> exportImagePixels(
    req::ptr<WandResource<MagickWand>> wand, int64_t x, int64_t y,
    int64_t width, int64_t height, const String& map) {
  size_t size = width * height * map.length();
  vector<typename StorageTypeCPPType<T>::T> ret(size);
  auto status = MagickExportImagePixels(
    wand->getWand(), x, y, width, height, map.c_str(), T, (void*)ret.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to export image pixels");
  }
  return ret;
}

static Array HHVM_METHOD(Imagick, exportImagePixels,
                         int64_t x, int64_t y,
                         int64_t width, int64_t height,
                         const String& map, int64_t storage_) {
  auto wand = getMagickWandResource(Object{this_});
  ensurePageIsValid(x, y, width, height);
  ensureChannelMapIsValid(map);
  auto storage = resolveStorageType((StorageType)storage_);

  if (storage == DoublePixel) {
    auto ret = exportImagePixels<DoublePixel>(wand, x, y, width, height, map);
    return convertArray(ret.size(), ret.data());
  } else {
    vector<int64_t> ret;
    if (storage == CharPixel) {
      auto tmp = exportImagePixels<CharPixel>(wand, x, y, width, height, map);
      ret.assign(tmp.begin(), tmp.end());
    } else {
      auto tmp = exportImagePixels<LongPixel>(wand, x, y, width, height, map);
      ret.assign(tmp.begin(), tmp.end());
    }
    return convertArray(ret.size(), ret.data());
  }
}

static bool HHVM_METHOD(Imagick, extentImage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickExtentImage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to extent image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, flattenImages) {
  raiseDeprecated(s_Imagick.c_str(), "flattenImages");

  auto wand = getMagickWandResource(Object{this_});
  MagickSetFirstIterator(wand->getWand());
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto magick = MagickFlattenImages(wand->getWand());
#pragma GCC diagnostic pop
  if (magick == nullptr) {
    IMAGICK_THROW("Flatten images failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, flipImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickFlipImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to flip image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, floodFillPaintImage,
                        const Variant& fill, double fuzz,
                        const Variant& target, int64_t x, int64_t y,
                        bool invert, int64_t channel /*=Default*/) {
  auto wand = getMagickWandResource(Object{this_});
  auto fillPixel = buildColorWand(fill);
  auto targetPixel = buildColorWand(target);
  auto status = MagickFloodfillPaintImage(
    wand->getWand(), (ChannelType)channel, fillPixel->getWand(),
    fuzz, targetPixel->getWand(), x, y, toMagickBool(invert));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to floodfill paint image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, flopImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickFlopImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to flop image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, frameImage, const Variant& matte_color,
    int64_t width, int64_t height, int64_t inner_bevel, int64_t outer_bevel) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(matte_color);
  auto status = MagickFrameImage(
    wand->getWand(), pixel->getWand(),
    width, height, inner_bevel, outer_bevel);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to frame image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, functionImage,
    int64_t func, const Array& arguments, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto args = toDoubleArray(arguments);
  if (args.empty()) {
    IMAGICK_THROW("The arguments array contains disallowed characters");
  }
  auto status = MagickFunctionImageChannel(
    wand->getWand(), (ChannelType)channel, (MagickFunction)func,
    args.size(), args.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to execute function on the image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, fxImage,
    const String& expression, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickFxImageChannel(
    wand->getWand(), (ChannelType)channel, expression.c_str());
  if (magick == nullptr) {
    IMAGICK_THROW("Fx image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, gammaImage, double gamma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickGammaImageChannel(
    wand->getWand(), (ChannelType)channel, gamma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to gamma image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, gaussianBlurImage,
    double radius, double sigma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickGaussianBlurImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to gaussian blur image");
  }
  return true;
}

static int64_t HHVM_METHOD(Imagick, getColorspace) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetColorspace(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getCompression) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetCompression(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getCompressionQuality) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetCompressionQuality(wand->getWand());
}

static String HHVM_STATIC_METHOD(Imagick, getCopyright) {
  return MagickGetCopyright();
}

static String HHVM_METHOD(Imagick, getFilename) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(MagickGetFilename(wand->getWand()));
}

static String HHVM_METHOD(Imagick, getFont) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(MagickGetFont(wand->getWand()));
}

static String HHVM_METHOD(Imagick, getFormat) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(MagickGetFormat(wand->getWand()));
}

static int64_t HHVM_METHOD(Imagick, getGravity) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetGravity(wand->getWand());
}

static String HHVM_STATIC_METHOD(Imagick, getHomeURL) {
  return convertMagickString(MagickGetHomeURL());
}

static Object HHVM_METHOD(Imagick, getImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickGetImage(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Get image failed");
  }
  return createImagick(magick);
}

static int64_t HHVM_METHOD(Imagick, getImageAlphaChannel) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageAlphaChannel(wand->getWand());
}

static String HHVM_METHOD(Imagick, getImageArtifact,
    const String& artifact) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(
    MagickGetImageArtifact(wand->getWand(), artifact.c_str()));
}

static Object HHVM_METHOD(Imagick, getImageBackgroundColor) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = newPixelWand();
  auto status = MagickGetImageBackgroundColor(
    wand->getWand(), pixel->getWand());
  if (pixel->getWand() == nullptr || status == MagickFalse) {
    IMAGICK_THROW("Unable to get image background color");
  }
  return createImagickPixel(pixel->releaseWand());
}

static String HHVM_METHOD(Imagick, getImageBlob) {
  auto wand = getMagickWandResource(Object{this_});
  ensureImageHasFormat(wand->getWand());
  size_t size;
  auto data = MagickGetImageBlob(wand->getWand(), &size);
  return convertMagickData(size, data);
}

static Array HHVM_METHOD(Imagick, getImageBluePrimary) {
  auto wand = getMagickWandResource(Object{this_});
  double x, y;
  auto status = MagickGetImageBluePrimary(wand->getWand(), &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image blue primary");
  }
  return make_dict_array(s_x, x, s_y, y);
}

static Object HHVM_METHOD(Imagick, getImageBorderColor) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = newPixelWand();
  auto status = MagickGetImageBorderColor(wand->getWand(), pixel->getWand());
  if (pixel->getWand() == nullptr || status == MagickFalse) {
    IMAGICK_THROW("Unable to get image border color");
  }
  return createImagickPixel(pixel->releaseWand());
}

static int64_t HHVM_METHOD(Imagick, getImageChannelDepth, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageChannelDepth(wand->getWand(), (ChannelType)channel);
}

static double HHVM_METHOD(Imagick, getImageChannelDistortion,
    const Object& reference, int64_t channel, int64_t metric) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(reference);
  double distortion;
  auto status = MagickGetImageChannelDistortion(
    wand->getWand(), wand2->getWand(),
    (ChannelType)channel, (MetricType)metric, &distortion);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image channel distortion");
  }
  return distortion;
}

static double HHVM_METHOD(Imagick, getImageChannelDistortions,
    const Object& reference, int64_t metric, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(reference);
  double distortion;
  auto status = MagickGetImageChannelDistortion(
    wand->getWand(), wand2->getWand(),
    (ChannelType)channel, (MetricType)metric, &distortion);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image channel distortion metrics");
  }
  return distortion;
}

static Array HHVM_METHOD(Imagick, getImageChannelExtrema, int64_t channel) {
  raiseDeprecated(s_Imagick.c_str(), "getImageChannelExtrema");

  auto wand = getMagickWandResource(Object{this_});
  size_t minima, maxima;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickGetImageChannelExtrema(
    wand->getWand(), (ChannelType)channel, &minima, &maxima);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image channel extrema");
  }
  return make_dict_array(
    s_minima, (int64_t)minima,
    s_maxima, (int64_t)maxima);
}

static Array HHVM_METHOD(Imagick, getImageChannelKurtosis, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  double kurtosis, skewness;
  auto status = MagickGetImageChannelKurtosis(
    wand->getWand(), (ChannelType)channel, &kurtosis, &skewness);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image channel kurtosis");
  }
  return make_dict_array(
    s_kurtosis, kurtosis,
    s_skewness, skewness);
}

static Array HHVM_METHOD(Imagick, getImageChannelMean, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  double mean, standardDeviation;
  auto status = MagickGetImageChannelMean(
    wand->getWand(), (ChannelType)channel, &mean, &standardDeviation);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image channel mean");
  }
  return make_dict_array(
    s_mean, mean,
    s_standardDeviation, standardDeviation);
}

static Array HHVM_METHOD(Imagick, getImageChannelRange, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  double minima, maxima;
  auto status = MagickGetImageChannelRange(
    wand->getWand(), (ChannelType)channel, &minima, &maxima);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get channel range");
  }
  return make_dict_array(
    s_minima, minima,
    s_maxima, maxima);
}

static Array HHVM_METHOD(Imagick, getImageChannelStatistics) {
  static const int channels[] = {
    UndefinedChannel, RedChannel, CyanChannel, GreenChannel, MagentaChannel,
    BlueChannel, YellowChannel, OpacityChannel, BlackChannel, MatteChannel
  };
  auto wand = getMagickWandResource(Object{this_});
  auto stat = MagickGetImageChannelStatistics(wand->getWand());

  DictInit ret(sizeof(channels) / sizeof(channels[0]));
  for (auto channel : channels) {
    ret.set(channel, make_dict_array(
        s_mean, stat[channel].mean,
        s_minima, stat[channel].minima,
        s_maxima, stat[channel].maxima,
        s_standardDeviation, stat[channel].standard_deviation,
        s_depth, (int64_t)stat[channel].depth));
  }
  freeMagickMemory(stat);
  return ret.toArray();
}

static Object HHVM_METHOD(Imagick, getImageClipMask) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickGetImageClipMask(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Unable to get image clip mask");
  }
  return createImagick(magick);
}

static Object HHVM_METHOD(Imagick, getImageColormapColor, int64_t index) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = newPixelWand();
  auto status = MagickGetImageColormapColor(
    wand->getWand(), index , pixel->getWand());
  if (pixel->getWand() == nullptr || status == MagickFalse) {
    IMAGICK_THROW("Unable to get image colormap color");
  }
  return createImagickPixel(pixel->releaseWand());
}

static int64_t HHVM_METHOD(Imagick, getImageColors) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageColors(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageColorspace) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageColorspace(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageCompose) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageCompose(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageCompression) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageCompression(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageDelay) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageDelay(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageDepth) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageDepth(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageDispose) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageDispose(wand->getWand());
}

static double HHVM_METHOD(Imagick, getImageDistortion,
    const Object& reference, int64_t metric) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(reference);
  double distortion;
  auto status = MagickGetImageDistortion(
    wand->getWand(), wand2->getWand(), (MetricType)metric, &distortion);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image distortion");
  }
  return distortion;
}

static Array HHVM_METHOD(Imagick, getImageExtrema) {
  raiseDeprecated(s_Imagick.c_str(), "getImageExtrema");

  auto wand = getMagickWandResource(Object{this_});
  size_t min, max;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickGetImageExtrema(wand->getWand(), &min, &max);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image extrema");
  }
  return make_dict_array(
    s_min, (int64_t)min,
    s_max, (int64_t)max);
}

static String HHVM_METHOD(Imagick, getImageFilename) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(MagickGetImageFilename(wand->getWand()));
}

static String HHVM_METHOD(Imagick, getImageFormat) {
  auto wand = getMagickWandResource(Object{this_});
  ensureImageHasFormat(wand->getWand());
  return convertMagickString(MagickGetImageFormat(wand->getWand()));
}

static double HHVM_METHOD(Imagick, getImageGamma) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageGamma(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImageGeometry) {
  auto wand = getMagickWandResource(Object{this_});
  return ImageGeometry(wand->getWand()).toArray();
}

static int64_t HHVM_METHOD(Imagick, getImageGravity) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageGravity(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImageGreenPrimary) {
  auto wand = getMagickWandResource(Object{this_});
  double x, y;
  auto status = MagickGetImageGreenPrimary(wand->getWand(), &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image green primary");
  }
  return make_dict_array(s_x, x, s_y, y);
}

static int64_t HHVM_METHOD(Imagick, getImageHeight) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageHeight(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImageHistogram) {
  auto wand = getMagickWandResource(Object{this_});
  size_t colors;
  auto hist = MagickGetImageHistogram(wand->getWand(), &colors);
  auto ret = createImagickPixelArray(colors, hist);
  freeMagickMemory(hist);
  return ret;
}

static int64_t HHVM_METHOD(Imagick, getImageIndex) {
  raiseDeprecated(s_Imagick.c_str(), "getImageindex");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return MagickGetImageIndex(wand->getWand());
#pragma GCC diagnostic pop
}

static int64_t HHVM_METHOD(Imagick, getImageInterlaceScheme) {
  raiseDeprecated(s_Imagick.c_str(), "getImageInterlaceScheme");

  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageInterlaceScheme(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageInterpolateMethod) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageInterpolateMethod(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageIterations) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageIterations(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageLength) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSizeType length;
  auto status = MagickGetImageLength(wand->getWand(), &length);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to acquire image length");
  }
  return length;
}

static bool HHVM_METHOD(Imagick, getImageMatte) {
  raiseDeprecated(s_Imagick.c_str(), "getImageMatte");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return MagickGetImageMatte(wand->getWand()) == MagickTrue;
#pragma GCC diagnostic pop
}

static Object HHVM_METHOD(Imagick, getImageMatteColor) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = newPixelWand();
  auto status = MagickGetImageMatteColor(wand->getWand(), pixel->getWand());
  if (pixel->getWand() == nullptr || status == MagickFalse) {
    IMAGICK_THROW("Unable to get image matte color");
  }
  return createImagickPixel(pixel->releaseWand());
}

static String HHVM_METHOD(Imagick, getImageMimeType) {
  auto wand = getMagickWandResource(Object{this_});
  return getImageMimeType(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageOrientation) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageOrientation(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImagePage) {
  auto wand = getMagickWandResource(Object{this_});
  size_t width, height;
  ssize_t x, y;
  auto status = MagickGetImagePage(wand->getWand(), &width, &height, &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image page");
  }
  return make_dict_array(
    s_width, (int64_t)width,
    s_height, (int64_t)height,
    s_x, (int64_t)x,
    s_y, (int64_t)y);
}

static Object HHVM_METHOD(Imagick, getImagePixelColor,
    int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = newPixelWand();
  auto status = MagickGetImagePixelColor(
    wand->getWand(), x, y , pixel->getWand());
  if (pixel->getWand() == nullptr || status == MagickFalse) {
    IMAGICK_THROW("Unable get image pixel color");
  }
  return createImagickPixel(pixel->releaseWand());
}

ALWAYS_INLINE
static String magickGetImageProfile(MagickWand* wand, const char* name) {
  size_t length;
  auto profile = MagickGetImageProfile(wand, name, &length);
  if (profile == nullptr) {
    IMAGICK_THROW("Can not get image profile");
  } else {
    return convertMagickData(length, profile);
  }
}

static String HHVM_METHOD(Imagick, getImageProfile, const String& name) {
  auto wand = getMagickWandResource(Object{this_});
  return magickGetImageProfile(wand->getWand(), name.c_str());
}

static Variant HHVM_METHOD(Imagick, getImageProfiles,
    const String& pattern, bool with_values) {
  auto wand = getMagickWandResource(Object{this_});
  size_t count;
  auto profiles = MagickGetImageProfiles(
    wand->getWand(), pattern.c_str(), &count);
  if (profiles == nullptr) {
    IMAGICK_THROW("Unable to get image profiles");
  }

  if (with_values) {
    DictInit ret(count);
    for (size_t i = 0; i < count; ++i) {
      ret.set(
        String(profiles[i]),
        magickGetImageProfile(wand->getWand(), profiles[i]));
    }
    freeMagickMemory(profiles);
    return ret.toArray();
  } else {
    return convertMagickArray(count, profiles);
  }
}

ALWAYS_INLINE
static String magickGetImageProperty(MagickWand* wand, const char* name) {
  return convertMagickString(MagickGetImageProperty(wand, name));
}

static Variant HHVM_METHOD(Imagick, getImageProperties,
    const String& pattern, bool with_values) {
  auto wand = getMagickWandResource(Object{this_});
  size_t count;
  auto properties = MagickGetImageProperties(
    wand->getWand(), pattern.c_str(), &count);
  if (properties == nullptr) {
    IMAGICK_THROW("Unable to get image properties");
  }

  if (with_values) {
    DictInit ret(count);
    for (size_t i = 0; i < count; ++i) {
      ret.set(
        String(properties[i]),
        magickGetImageProperty(wand->getWand(), properties[i]));
    }
    freeMagickMemory(properties);
    return ret.toArray();
  } else {
    return convertMagickArray(count, properties);
  }
}

static String HHVM_METHOD(Imagick, getImageProperty, const String& name) {
  auto wand = getMagickWandResource(Object{this_});
  return magickGetImageProperty(wand->getWand(), name.c_str());
}

static Array HHVM_METHOD(Imagick, getImageRedPrimary) {
  auto wand = getMagickWandResource(Object{this_});
  double x, y;
  auto status = MagickGetImageRedPrimary(wand->getWand(), &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image red primary");
  }
  return make_dict_array(s_x, x, s_y, y);
}

static Object HHVM_METHOD(Imagick, getImageRegion,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickGetImageRegion(wand->getWand(), width, height, x, y);
  if (magick == nullptr) {
    IMAGICK_THROW("Get image region failed");
  }
  return createImagick(magick);
}

static int64_t HHVM_METHOD(Imagick, getImageRenderingIntent) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageRenderingIntent(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImageResolution) {
  auto wand = getMagickWandResource(Object{this_});
  double x, y;
  auto status = MagickGetImageResolution(wand->getWand(), &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image resolution");
  }
  return make_dict_array(s_x, x, s_y, y);
}

static String HHVM_METHOD(Imagick, getImagesBlob) {
  auto wand = getMagickWandResource(Object{this_});
  int current = MagickGetIteratorIndex(wand->getWand());
  MagickResetIterator(wand->getWand());
  while (MagickNextImage(wand->getWand()) != MagickFalse) {
    ensureImageHasFormat(wand->getWand());
  }
  auto status = MagickSetIteratorIndex(wand->getWand(), current);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set the iterator index");
  }
  size_t size;
  auto data = MagickGetImagesBlob(wand->getWand(), &size);
  return convertMagickData(size, data);
}

static int64_t HHVM_METHOD(Imagick, getImageScene) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageScene(wand->getWand());
}

static String HHVM_METHOD(Imagick, getImageSignature) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageSignature(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageSize) {
  raiseDeprecated(s_Imagick.c_str(), "getImageSize",
                  s_Imagick.c_str(), "getImageLength");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return MagickGetImageSize(wand->getWand());
#pragma GCC diagnostic pop
}

static int64_t HHVM_METHOD(Imagick, getImageTicksPerSecond) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageTicksPerSecond(wand->getWand());
}

static double HHVM_METHOD(Imagick, getImageTotalInkDensity) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageTotalInkDensity(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageType) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageType(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageUnits) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageUnits(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getImageVirtualPixelMethod) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageVirtualPixelMethod(wand->getWand());
}

static Array HHVM_METHOD(Imagick, getImageWhitePoint) {
  auto wand = getMagickWandResource(Object{this_});
  double x, y;
  auto status = MagickGetImageWhitePoint(wand->getWand(), &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get image white point");
  }
  return make_dict_array(s_x, x, s_y, y);
}

static int64_t HHVM_METHOD(Imagick, getImageWidth) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetImageWidth(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getInterlaceScheme) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetInterlaceScheme(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getIteratorIndex) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetIteratorIndex(wand->getWand());
}

static int64_t HHVM_METHOD(Imagick, getNumberImages) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetNumberImages(wand->getWand());
}

static String HHVM_METHOD(Imagick, getOption, const String& key) {
  auto wand = getMagickWandResource(Object{this_});
  return convertMagickString(MagickGetOption(wand->getWand(), key.c_str()));
}

static String HHVM_STATIC_METHOD(Imagick, getPackageName) {
  return MagickGetPackageName();
}

static Array HHVM_METHOD(Imagick, getPage) {
  auto wand = getMagickWandResource(Object{this_});
  size_t width, height;
  ssize_t x, y;
  auto status = MagickGetPage(wand->getWand(), &width, &height, &x, &y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get page");
  }
  return make_dict_array(
    s_width, (int64_t)width,
    s_height, (int64_t)height,
    s_x, (int64_t)x,
    s_y, (int64_t)y);
}

static Object HHVM_METHOD(Imagick, getPixelIterator) {
  return createPixelIterator(Object{this_});
}

static Object HHVM_METHOD(Imagick, getPixelRegionIterator,
    int64_t x, int64_t y, int64_t columns, int64_t rows) {
  return createPixelRegionIterator(Object{this_}, x, y, columns, rows);
}

static double HHVM_METHOD(Imagick, getPointSize) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickGetPointsize(wand->getWand());
}

static Array HHVM_STATIC_METHOD(Imagick, getQuantumDepth) {
  size_t depth;
  const char* quantumDepth = MagickGetQuantumDepth(&depth);
  return make_dict_array(
    s_quantumDepthLong, (int64_t)depth,
    s_quantumDepthString, quantumDepth);
}

static Array HHVM_STATIC_METHOD(Imagick, getQuantumRange) {
  size_t range;
  const char* quantumRange = MagickGetQuantumRange(&range);
  return make_dict_array(
    s_quantumRangeLong, (int64_t)range,
    s_quantumRangeString, quantumRange);
}

static String HHVM_STATIC_METHOD(Imagick, getReleaseDate) {
  return MagickGetReleaseDate();
}

static int64_t HHVM_STATIC_METHOD(Imagick, getResource,
    int64_t resource_type) {
  return MagickGetResource((ResourceType)resource_type);
}

static int64_t HHVM_STATIC_METHOD(Imagick, getResourceLimit,
    int64_t resource_type) {
  return MagickGetResourceLimit((ResourceType)resource_type);
}

static Array HHVM_METHOD(Imagick, getSamplingFactors) {
  auto wand = getMagickWandResource(Object{this_});
  size_t num;
  auto arr = MagickGetSamplingFactors(wand->getWand(), &num);
  return convertMagickArray(num, arr);
}

static Array HHVM_METHOD(Imagick, getSize) {
  auto wand = getMagickWandResource(Object{this_});
  size_t columns, rows;
  auto status = MagickGetSize(wand->getWand(), &columns, &rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get size");
  }
  return make_dict_array(
    s_columns, (int64_t)columns,
    s_rows, (int64_t)rows);
}

static int64_t HHVM_METHOD(Imagick, getSizeOffset) {
  auto wand = getMagickWandResource(Object{this_});
  ssize_t offset;
  auto status = MagickGetSizeOffset(wand->getWand(), &offset);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get size offset");
  }
  return offset;
}

static Array HHVM_STATIC_METHOD(Imagick, getVersion) {
  size_t version;
  const char* versionStr = MagickGetVersion(&version);
  return make_dict_array(
    s_versionNumber, (int64_t)version,
    s_versionString, versionStr);
}

static bool HHVM_METHOD(Imagick, haldClutImage,
    const Object& clut, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(clut);
  auto status = MagickHaldClutImageChannel(
    wand->getWand(), (ChannelType)channel, wand2->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to hald clut image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, hasNextImage) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickHasNextImage(wand->getWand()) != MagickFalse;
}

static bool HHVM_METHOD(Imagick, hasPreviousImage) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickHasPreviousImage(wand->getWand()) != MagickFalse;
}

ALWAYS_INLINE
static vector<pair<String, String>> parseIdentify(const String& identify) {
  using Keys = vector<pair<string, StaticString>>;
  static const Keys keys = {
    {"Format: ", s_format},
    {"Units: ", s_units},
    {"Type: ", s_type},
    {"Colorspace: ", s_colorSpace},
    {"Filesize: ", s_fileSize},
    {"Compression: ", s_compression}
  };

  vector<pair<String, String>> ret;
  Array lines = HHVM_FN(explode)("\r\n", identify).toArray();
  ret.reserve(keys.size());
  for (ArrayIter it(lines); it; ++it) {
    String line = HHVM_FN(trim)(
      tvCastToString(it.secondValPlus())
    );
    auto key = std::find_if(keys.begin(), keys.end(),
        [=](Keys::const_reference i) {
          const string& prefix = i.first;
          return strncmp(line.c_str(), prefix.c_str(), prefix.length()) == 0;
        });
    if (key != keys.end()) {
      ret.push_back({key->second, line.substr(key->first.length())});
    }
  }
  return ret;
}

static Array HHVM_METHOD(Imagick, identifyImage, bool appendRawOutput) {
  static const StaticString s_unknown("unknown");
  auto wand = getMagickWandResource(Object{this_});
  String identify = convertMagickString(MagickIdentifyImage(wand->getWand()));
  auto parsedIdentify = parseIdentify(identify);
  DictInit ret(parsedIdentify.size() + 6);

  ret.set(s_imageName,
    convertMagickString(MagickGetImageFilename(wand->getWand())));

  String mimetype = HHVM_MN(Imagick, getImageMimeType)(this_);
  ret.set(s_mimetype, mimetype.empty() ? String(s_unknown) : mimetype);

  for (const auto& i: parsedIdentify) {
    ret.set(i.first, i.second);
  }

  ret.set(s_geometry, ImageGeometry(wand->getWand()).toArray());

  double x, y;
  if (MagickGetImageResolution(wand->getWand(), &x, &y) == MagickTrue) {
    ret.set(s_resolution, make_dict_array(s_x, x, s_y, y));
  }

  ret.set(s_signature,
    convertMagickString(MagickGetImageSignature(wand->getWand())));

  if (appendRawOutput) {
    ret.set(s_rawOutput, identify);
  }

  return ret.toArray();
}

static bool HHVM_METHOD(Imagick, implodeImage, double radius) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickImplodeImage(wand->getWand(), radius);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to implode image");
  }
  return true;
}

template<StorageType T>
ALWAYS_INLINE
static void importImagePixels(req::ptr<WandResource<MagickWand>> wand,
    int64_t x, int64_t y, int64_t width, int64_t height,
    const String& map, const vector<double>& array) {
  vector<typename StorageTypeCPPType<T>::T> data(array.begin(), array.end());
  auto status = MagickImportImagePixels(wand->getWand(),
    x, y, width, height, map.c_str(), T, data.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to import image pixels");
  }
}

static bool HHVM_METHOD(Imagick, importImagePixels,
                        int64_t x, int64_t y, int64_t width, int64_t height,
                        const String& map, int64_t storage_,
                        const Array& pixels) {
  auto wand = getMagickWandResource(Object{this_});
  ensurePageIsValid(x, y, width, height);
  ensureChannelMapIsValid(map);
  if (pixels.size() != width * height * map.size()) {
    IMAGICK_THROW("The array contains incorrect number of elements");
  }
  auto array = toDoubleArray(pixels);
  if (array.empty()) {
    IMAGICK_THROW("The array contains incorrect values");
  }
  auto storage = resolveStorageType((StorageType)storage_);

  if (storage == CharPixel) {
    importImagePixels<CharPixel>(wand, x, y, width, height, map, array);
  } else if (storage == LongPixel) {
    importImagePixels<LongPixel>(wand, x, y, width, height, map, array);
  } else if (storage == DoublePixel) {
    importImagePixels<DoublePixel>(wand, x, y, width, height, map, array);
  } else {
    not_reached();
  }
  return true;
}

static bool HHVM_METHOD(Imagick, labelImage, const String& label) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickLabelImage(wand->getWand(), label.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to label image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, levelImage,
    double blackPoint, double gamma, double whitePoint, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickLevelImageChannel(
    wand->getWand(), (ChannelType)channel, blackPoint, gamma, whitePoint);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to level image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, linearStretchImage,
    double blackPoint, double whitePoint) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickLinearStretchImage(
    wand->getWand(), blackPoint, whitePoint);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to linear strech image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, liquidRescaleImage,
    int64_t width, int64_t height, double delta_x, double rigidity) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickLiquidRescaleImage(
    wand->getWand(), width, height, delta_x, rigidity);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to liquid rescale image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, magnifyImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickMagnifyImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to magnify image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, mapImage, const Object& map, bool dither) {
  raiseDeprecated(s_Imagick.c_str(), "mapImage");

  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(map);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickMapImage(
    wand->getWand(), wand2->getWand(), toMagickBool(dither));
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to map image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, matteFloodfillImage,
    double alpha, double fuzz, const Variant& bordercolor,
    int64_t x, int64_t y) {
  raiseDeprecated(s_Imagick.c_str(), "matteFloodfillImage");

  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(bordercolor);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickMatteFloodfillImage(
    wand->getWand(), alpha, fuzz, pixel->getWand(), x, y);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to matte floodfill image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, medianFilterImage, double radius) {
  raiseDeprecated(s_Imagick.c_str(), "medianFilterImage");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickMedianFilterImage(wand->getWand(), radius);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to median filter image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, mergeImageLayers, int64_t layer_method) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSetFirstIterator(wand->getWand());
  auto magick = MagickMergeImageLayers(
    wand->getWand(), (ImageLayerMethod)layer_method);
  if (magick == nullptr) {
    IMAGICK_THROW("Unable to merge image layers");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, minifyImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickMinifyImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to minify image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, modulateImage,
    double brightness, double saturation, double hue) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickModulateImage(
    wand->getWand(), brightness, saturation, hue);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to modulate image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, montageImage, const Object& draw,
    const String& tile_geometry, const String& thumbnail_geometry,
    int64_t montage_mode, const String& frame) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(draw);
  auto magick = MagickMontageImage(
    wand->getWand(), drawing->getWand(),
    tile_geometry.c_str(), thumbnail_geometry.c_str(),
    (MontageMode)montage_mode, frame.c_str());
  if (magick == nullptr) {
    IMAGICK_THROW("Montage image failed");
  }
  return createImagick(magick);
}

static Object HHVM_METHOD(Imagick, morphImages, int64_t number_frames) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickMorphImages(wand->getWand(), number_frames);
  if (magick == nullptr) {
    IMAGICK_THROW("Morphing images failed");
  }
  return createImagick(magick);
}

static Object HHVM_METHOD(Imagick, mosaicImages) {
  raiseDeprecated(s_Imagick.c_str(), "mosaicImages");

  auto wand = getMagickWandResource(Object{this_});
  MagickSetFirstIterator(wand->getWand());
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto magick = MagickMosaicImages(wand->getWand());
#pragma GCC diagnostic pop
  if (magick == nullptr) {
    IMAGICK_THROW("Mosaic image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, motionBlurImage,
    double radius, double sigma, double angle, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickMotionBlurImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma, angle);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to motion blur image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, negateImage, bool gray, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickNegateImageChannel(
    wand->getWand(), (ChannelType)channel, toMagickBool(gray));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to negate image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, newImage,
    int64_t cols, int64_t rows, const Variant& background,
    const String& format) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(background);
  auto status = MagickNewImage(wand->getWand(), cols, rows, pixel->getWand());
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

static bool HHVM_METHOD(Imagick, newPseudoImage,
    int64_t columns, int64_t rows, const String& pseudoString) {
  auto wand = getMagickWandResource(Object{this_});
  if (!isMagickPseudoFormat(pseudoString)) {
    IMAGICK_THROW("Invalid pseudo format string");
  }
  auto status = MagickSetSize(wand->getWand(), columns, rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to create new pseudo image");
  }
  imagickReadOp(wand->getWand(), pseudoString, MagickReadImage);
  return true;
}

static bool HHVM_METHOD(Imagick, nextImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickNextImage(wand->getWand());
  setImagePending(Object{this_}, status == MagickFalse);
  return status != MagickFalse;
}

static bool HHVM_METHOD(Imagick, normalizeImage, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickNormalizeImageChannel(
    wand->getWand(), (ChannelType)channel);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to normalize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, oilPaintImage, double radius) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickOilPaintImage(wand->getWand(), radius);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to oilpaint image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, opaquePaintImage,
    const Variant& target, const Variant& fill,
    double fuzz, bool invert, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto fillPixel = buildColorWand(fill);
  auto targetPixel = buildColorWand(target);
  auto status = MagickOpaquePaintImageChannel(
    wand->getWand(), (ChannelType)channel, fillPixel->getWand(),
    targetPixel->getWand(), fuzz, toMagickBool(invert));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to opaque paint image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, optimizeImageLayers) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickOptimizeImageLayers(wand->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Optimize image layers failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, orderedPosterizeImage,
    const String& threshold_map, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickOrderedPosterizeImageChannel(
    wand->getWand(), (ChannelType)channel, threshold_map.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to posterize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, paintFloodfillImage,
                        const Variant& fill, double fuzz,
                        const Variant& bordercolor,
                        int64_t x, int64_t y, int64_t channel) {
  raiseDeprecated(s_Imagick.c_str(), "paintFloodfillImage");
  auto wand = getMagickWandResource(Object{this_});
  auto fillPixel = buildColorWand(fill);
  auto borderPixel = bordercolor.isNull()
                   ? req::make<WandResource<PixelWand>>(nullptr)
                   : buildColorWand(bordercolor);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickPaintFloodfillImage(
    wand->getWand(), (ChannelType)channel,
    fillPixel->getWand(), fuzz, borderPixel->getWand(), x, y);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to paint floodfill image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, paintOpaqueImage,
    const Variant& target, const Variant& fill,
    double fuzz, int64_t channel) {
  raiseDeprecated(s_Imagick.c_str(), "paintOpaqueImage");

  auto wand = getMagickWandResource(Object{this_});
  auto fillPixel = buildColorWand(fill);
  auto targetPixel = buildColorWand(target);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickPaintOpaqueImageChannel(
    wand->getWand(), (ChannelType)channel,
    targetPixel->getWand(), fillPixel->getWand(), fuzz);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable paint opaque image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, paintTransparentImage,
    const Variant& target, double alpha, double fuzz) {
  raiseDeprecated(s_Imagick.c_str(), "paintTransparentImage");

  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(target);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickPaintTransparentImage(
    wand->getWand(), pixel->getWand(), alpha, fuzz);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to paint transparent image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, pingImage, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  imagickReadOp(wand->getWand(), filename, MagickPingImage);
  return true;
}

static bool HHVM_METHOD(Imagick, pingImageBlob, const String& image) {
  auto wand = getMagickWandResource(Object{this_});
  if (image.empty()) {
    IMAGICK_THROW("Empty image string passed");
  }
  auto status = MagickPingImageBlob(
    wand->getWand(), image.c_str(), image.size());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to ping image blob");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, pingImageFile, const OptResource& filehandle,
                        const String& /*fileName*/) {
  auto wand = getMagickWandResource(Object{this_});
  imagickReadOp(wand->getWand(), filehandle, MagickPingImageFile);
  return true;
}

static bool HHVM_METHOD(Imagick, polaroidImage,
    const Object& properties, double angle) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(properties);
  auto status = MagickPolaroidImage(
    wand->getWand(), drawing->getWand(), angle);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to polaroid image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, posterizeImage,
    int64_t levels, bool dither) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickPosterizeImage(
    wand->getWand(), levels, toMagickBool(dither));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to posterize image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, previewImages, int64_t preview) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickPreviewImages(wand->getWand(), (PreviewType)preview);
  if (magick == nullptr) {
    IMAGICK_THROW("Preview images failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, previousImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickPreviousImage(wand->getWand());
  setImagePending(Object{this_}, false);
  return status != MagickFalse;
}

static bool HHVM_METHOD(Imagick, profileImage,
    const String& name, const String& profile) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickProfileImage(
    wand->getWand(), name.c_str(), profile.c_str(), profile.length());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to profile image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, quantizeImage,
    int64_t numberColors, int64_t colorspace,
    int64_t treedepth, bool dither, bool measureError) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickQuantizeImage(
    wand->getWand(), numberColors, (ColorspaceType)colorspace, treedepth,
    toMagickBool(dither), toMagickBool(measureError));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to quantize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, quantizeImages,
    int64_t numberColors, int64_t colorspace,
    int64_t treedepth, bool dither, bool measureError) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickQuantizeImages(
    wand->getWand(), numberColors, (ColorspaceType)colorspace, treedepth,
    toMagickBool(dither), toMagickBool(measureError));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to quantize images");
  }
  return true;
}

static Array HHVM_METHOD(Imagick, queryFontMetrics,
    const Object& properties, const String& text,
    const Variant& query_multiline) {
  auto wand = getMagickWandResource(Object{this_});
  auto drawing = getDrawingWandResource(properties);

  // No parameter passed, this means we should autodetect
  bool multiline = query_multiline.isNull()
                 ? text.find('\n') != String::npos
                 : query_multiline.toBoolean();

  // wand is empty, create a 1x1 pixel image to use as a temporary canvas
  bool removeCanvas;
  if (MagickGetNumberImages(wand->getWand()) < 1) {
    auto pixel = newPixelWand();
    if (pixel->getWand() == nullptr) {
      IMAGICK_THROW(
        "Unable to allocate background color for the temporary canvas");
    }
    auto status = MagickNewImage(wand->getWand(), 1, 1, pixel->getWand());
    if (status == MagickFalse) {
      IMAGICK_THROW("Unable to allocate temporary canvas");
    }
    removeCanvas = true;
  } else {
    removeCanvas = false;
  }

  // Multiline testing
  double* metrics;
  if (multiline) {
    metrics = MagickQueryMultilineFontMetrics(
      wand->getWand(), drawing->getWand(), text.c_str());
  } else {
    metrics = MagickQueryFontMetrics(
      wand->getWand(), drawing->getWand(), text.c_str());
  }

  // Remove the image from the stack
  if (removeCanvas) {
    MagickRemoveImage(wand->getWand());
  }

  if (metrics == nullptr) {
    IMAGICK_THROW("Failed to query the font metrics");
  } else {
    static const StaticString keys[] = {
      s_characterWidth, s_characterHeight, s_ascender, s_descender,
      s_textWidth, s_textHeight, s_maxHorizontalAdvance,
      // s_x1, s_y1, s_x2, s_y2,
      s_boundingBox, empty_string_ref, empty_string_ref, empty_string_ref,
      s_originX, s_originY
    };
    static const size_t boundingBoxOffset = 7;
    static const size_t size = 13;

    DictInit ret(size - 3);
    for (size_t i = 0; i < size; ++i) {
      if (keys[i] == s_boundingBox) {
        ret.set(s_boundingBox, make_dict_array(
                s_x1, metrics[boundingBoxOffset + 0],
                s_y1, metrics[boundingBoxOffset + 1],
                s_x2, metrics[boundingBoxOffset + 2],
                s_y2, metrics[boundingBoxOffset + 3]));
      } else if (!keys[i].empty()) {
        ret.set(keys[i], metrics[i]);
      }
    }
    freeMagickMemory(metrics);
    return ret.toArray();
  }
}

static Array HHVM_STATIC_METHOD(Imagick, queryFonts,
    const String& pattern) {
  return magickQueryFonts(pattern.c_str());
}

static Array HHVM_STATIC_METHOD(Imagick, queryFormats,
    const String& pattern) {
  return magickQueryFormats(pattern.c_str());
}

static bool HHVM_METHOD(Imagick, radialBlurImage,
    double angle, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickRadialBlurImageChannel(
    wand->getWand(), (ChannelType)channel, angle);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to radial blur image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, raiseImage,
    int64_t width, int64_t height, int64_t x, int64_t y, bool raise) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickRaiseImage(
    wand->getWand(), width, height, x, y, toMagickBool(raise));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to raise image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, randomThresholdImage,
    double low, double high, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickRandomThresholdImageChannel(
    wand->getWand(), (ChannelType)channel, low, high);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to random threshold image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, readImage, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  imagickReadOp(wand->getWand(), filename, MagickReadImage);
  return true;
}

static bool HHVM_METHOD(Imagick, readImageBlob,
    const String& image, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickReadImageBlob(
    wand->getWand(), image.c_str(), image.size());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to read image blob");
  }
  if (!filename.empty()) {
    MagickSetImageFilename(wand->getWand(), filename.c_str());
  }
  MagickSetLastIterator(wand->getWand());
  return true;
}

static bool HHVM_METHOD(Imagick, readImageFile,
    const OptResource& filehandle, const String& fileName) {
  auto wand = getMagickWandResource(Object{this_});
  imagickReadOp(wand->getWand(), filehandle, MagickReadImageFile);
  if (!fileName.empty()) {
    MagickSetImageFilename(wand->getWand(), fileName.c_str());
  }
  MagickSetLastIterator(wand->getWand());
  return true;
}

static bool HHVM_METHOD(Imagick, readImages, const Array& files) {
  auto wand = getMagickWandResource(Object{this_});
  for (ArrayIter it(files); it; ++it) {
    imagickReadOp(
      wand->getWand(),
      tvCastToString(it.secondValPlus()),
      MagickReadImage
    );
  }
  return true;
}

static bool HHVM_METHOD(Imagick, recolorImage, const Array& matrix) {
  raiseDeprecated(s_Imagick.c_str(), "recolorImage");

  auto wand = getMagickWandResource(Object{this_});
  auto array = toDoubleArray(matrix);
  if (array.empty()) {
    IMAGICK_THROW("The map contains disallowed characters");
  }
  auto order = (size_t)sqrt(array.size());
  if (order * order != array.size()) {
    IMAGICK_THROW("The color matrix must contain a square number of elements");
  }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickRecolorImage(wand->getWand(), order, array.data());
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to recolor image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, reduceNoiseImage, double radius) {
  raiseDeprecated(s_Imagick.c_str(), "reduceNoiseImage");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickReduceNoiseImage(wand->getWand(), radius);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to reduce image noise");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, remapImage,
    const Object& replacement, int64_t dither) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(replacement);
  auto status = MagickRemapImage(
    wand->getWand(), wand2->getWand(), (DitherMethod)dither);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to remap image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, removeImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickRemoveImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to remove image");
  }
  MagickSetLastIterator(wand->getWand());
  setImagePending(Object{this_}, true);
  return true;
}

static String HHVM_METHOD(Imagick, removeImageProfile, const String& name) {
  auto wand = getMagickWandResource(Object{this_});
  size_t length;
  auto profile = MagickRemoveImageProfile(
    wand->getWand(), name.c_str(), &length);
  if (profile == nullptr) {
    IMAGICK_THROW("The image profile does not exist");
  } else {
    return convertMagickData(length, profile);
  }
}

static bool HHVM_METHOD(Imagick, resampleImage,
    double x_resolution, double y_resolution, int64_t filter, double blur) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickResampleImage(
    wand->getWand(), x_resolution, y_resolution, (FilterTypes)filter, blur);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to resample image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, resetImagePage, const String& page) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickResetImagePage(wand->getWand(), page.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to reset image page");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, resizeImage,
    int64_t columns, int64_t rows,
    int64_t filter, double blur, bool bestfit) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand()).toThumbnail(
    {columns, rows}, bestfit);
  if (geometry.empty()) {
    IMAGICK_THROW("Invalid image geometry");
  }
  auto status = MagickResizeImage(
    wand->getWand(), geometry.getWidth(), geometry.getHeight(),
    (FilterTypes)filter, blur);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to resize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, rollImage, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickRollImage(wand->getWand(), x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to roll image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, rotateImage,
    const Variant& background, double degrees) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(background);
  auto status = MagickRotateImage(wand->getWand(), pixel->getWand(), degrees);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to rotate image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, roundCornersImage,
    double x_rounding, double y_rounding,
    double stroke_width, double displace, double size_correction) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand());

  if (geometry.empty()) {
    IMAGICK_THROW("Unable to round corners on empty image");
  }

  if (MagickSetImageMatte(wand->getWand(), MagickTrue) == MagickFalse) {
    IMAGICK_THROW("Unable to set image matte");
  }

  auto pixel = newPixelWand();
  if (pixel->getWand() == nullptr) {
    IMAGICK_THROW("Failed to allocate PixelWand structure");
  }

  auto drawing = req::make<WandResource<DrawingWand>>(NewDrawingWand());
  if (drawing->getWand() == nullptr) {
    IMAGICK_THROW("Failed to allocate DrawingWand structure");
  }

  auto magick = req::make<WandResource<MagickWand>>(NewMagickWand());
  if (magick->getWand() == nullptr) {
    IMAGICK_THROW("Failed to allocate MagickWand structure");
  }

  if (PixelSetColor(pixel->getWand(), "transparent") == MagickFalse) {
    IMAGICK_THROW("Unable to set pixel color");
  }

  if (MagickNewImage(magick->getWand(),
                     geometry.getWidth(),
                     geometry.getHeight(),
                     pixel->getWand()) == MagickFalse) {
    IMAGICK_THROW("Unable to allocate mask image");
  }
  MagickSetImageBackgroundColor(magick->getWand(), pixel->getWand());

  if (PixelSetColor(pixel->getWand(), "white") == MagickFalse) {
    IMAGICK_THROW("Unable to set pixel color");
  }
  DrawSetFillColor(drawing->getWand(), pixel->getWand());

  if (PixelSetColor(pixel->getWand(), "black") == MagickFalse) {
    IMAGICK_THROW("Unable to set pixel color");
  }
  DrawSetStrokeColor(drawing->getWand(), pixel->getWand());
  DrawSetStrokeWidth(drawing->getWand(), stroke_width);
  DrawRoundRectangle(drawing->getWand(), displace, displace,
                     geometry.getWidth() + size_correction,
                     geometry.getHeight() + size_correction,
                     x_rounding, y_rounding);

  auto status = withMagickLocaleFix([&magick, &drawing](){
    return MagickDrawImage(magick->getWand(), drawing->getWand());
  });
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to draw on image");
  }

  if (MagickCompositeImage(wand->getWand(), magick->getWand(),
                           DstInCompositeOp, 0, 0) == MagickFalse) {
    IMAGICK_THROW("Unable to composite image");
  }

  return true;
}

static bool HHVM_METHOD(Imagick, roundCorners,
    double x_rounding, double y_rounding,
    double stroke_width, double displace, double size_correction) {
  // raiseDeprecated(s_Imagick.c_str(), "roundCorners",
  //                 s_Imagick.c_str(), "roundCornersImage");
  return HHVM_MN(Imagick, roundCornersImage)(
    this_, x_rounding, y_rounding,
    stroke_width, displace, size_correction);
}

static bool HHVM_METHOD(Imagick, sampleImage,
    int64_t columns, int64_t rows) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSampleImage(wand->getWand(), columns, rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sample image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, scaleImage,
    int64_t cols, int64_t rows, bool bestfit) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand()).toThumbnail(
    {cols, rows}, bestfit);
  if (geometry.empty()) {
    IMAGICK_THROW("Invalid image geometry");
  }
  auto status = MagickScaleImage(
    wand->getWand(), geometry.getWidth(), geometry.getHeight());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to scale image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, segmentImage, int64_t colorspace,
    double cluster_threshold, double smooth_threshold, bool verbose) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSegmentImage(
    wand->getWand(), (ColorspaceType)colorspace,
    toMagickBool(verbose), cluster_threshold, smooth_threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to segment image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, separateImageChannel, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSeparateImageChannel(
    wand->getWand(), (ChannelType)channel);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to separate image channel");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, sepiaToneImage, double threshold) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSepiaToneImage(wand->getWand(), threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sepia tone image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setBackgroundColor,
    const Variant& background) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(background);
  auto status = MagickSetBackgroundColor(wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set background color");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setColorspace, int64_t colorspace) {
  auto wand = getMagickWandResource(Object{this_});
  return MagickSetColorspace(
    wand->getWand(), (ColorspaceType)colorspace) != MagickFalse;
}

static bool HHVM_METHOD(Imagick, setCompression, int64_t compression) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetCompression(
    wand->getWand(), (CompressionType)compression);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set compression");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setCompressionQuality, int64_t quality) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetCompressionQuality(wand->getWand(), quality);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set compression quality");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setFilename, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetFilename(wand->getWand(), filename.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set filename");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setFirstIterator) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSetFirstIterator(wand->getWand());
  setImagePending(Object{this_}, false);
  return true;
}

static bool HHVM_METHOD(Imagick, setFont, const String& fontName) {
  auto wand = getMagickWandResource(Object{this_});
  auto font = magickResolveFont(fontName);
  if (font.isNull() ||
      MagickSetFont(wand->getWand(), font.c_str()) == MagickFalse) {
    IMAGICK_THROW("Unable to set font");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setFormat, const String& format) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetFormat(wand->getWand(), format.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set format");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setGravity, int64_t gravity) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetGravity(wand->getWand(), (GravityType)gravity);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set gravity");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImage, const Object& replace) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(replace);
  auto status = MagickSetImage(wand->getWand(), wand2->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set the image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageAlphaChannel, int64_t mode) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageAlphaChannel(
    wand->getWand(), (AlphaChannelType)mode);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image alpha channel");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageArtifact,
    const String& artifact, const String& value) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageArtifact(
    wand->getWand(), artifact.c_str(), value.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image artifact");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageBackgroundColor,
    const Variant& background) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(background);
  auto status = MagickSetImageBackgroundColor(
    wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image background color");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageBias, double bias) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageBias(wand->getWand(), bias);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image bias");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageBluePrimary, double x, double y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageBluePrimary(wand->getWand(), x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image blue primary");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageBorderColor, const Variant& border) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(border);
  auto status = MagickSetImageBorderColor(wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image border color");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageChannelDepth,
    int64_t channel, int64_t depth) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageChannelDepth(
    wand->getWand(), (ChannelType)channel, depth);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image channel depth");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageClipMask, const Object& clip_mask) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(clip_mask);
  auto status = MagickSetImageClipMask(wand->getWand(), wand2->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image clip mask");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageColormapColor,
    int64_t index, const Object& color) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(color);
  auto status = MagickSetImageColormapColor(
    wand->getWand(), index, pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image color map color");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageColorspace, int64_t colorspace) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageColorspace(
    wand->getWand(), (ColorspaceType)colorspace);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image colorspace");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageCompose, int64_t compose) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageCompose(
    wand->getWand(), (CompositeOperator)compose);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image composite operator");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageCompression, int64_t compression) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageCompression(
    wand->getWand(), (CompressionType)compression);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image compression");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageCompressionQuality,
    int64_t quality) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageCompressionQuality(wand->getWand(), quality);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image compression quality");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageDelay, int64_t delay) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageDelay(wand->getWand(), delay);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image delay");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageDepth, int64_t depth) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageDepth(wand->getWand(), depth);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image depth");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageDispose, int64_t dispose) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageDispose(
    wand->getWand(), (DisposeType)dispose);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image dispose");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageExtent,
    int64_t columns, int64_t rows) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageExtent(wand->getWand(), columns, rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image extent");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageFilename, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageFilename(wand->getWand(), filename.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image filename");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageFormat, const String& format) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageFormat(wand->getWand(), format.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image format");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageGamma, double gamma) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageGamma(wand->getWand(), gamma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image gamma");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageGravity, int64_t gravity) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageGravity(
    wand->getWand(), (GravityType)gravity);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image gravity");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageGreenPrimary, double x, double y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageGreenPrimary(wand->getWand(), x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image green primary");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageIndex, int64_t index) {
  raiseDeprecated(s_Imagick.c_str(), "setImageIndex");

  auto wand = getMagickWandResource(Object{this_});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto status = MagickSetImageIndex(wand->getWand(), index);
#pragma GCC diagnostic pop
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image index");
  }
  setImagePending(Object{this_}, false);
  return true;
}

static bool HHVM_METHOD(Imagick, setImageInterlaceScheme,
    int64_t interlace_scheme) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageInterlaceScheme(
    wand->getWand(), (InterlaceType)interlace_scheme);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image interlace scheme");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageInterpolateMethod, int64_t method) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageInterpolateMethod(
    wand->getWand(), (InterpolatePixelMethod)method);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set the image interpolate method");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageIterations, int64_t iterations) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageIterations(wand->getWand(), iterations);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image iterations");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageMatte, bool matte) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageMatte(wand->getWand(), toMagickBool(matte));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image matte");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageMatteColor, const Variant& matte) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(matte);
  auto status = MagickSetImageMatteColor(wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image matte color");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageOpacity, double opacity) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageOpacity(wand->getWand(), opacity);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image opacity");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageOrientation, int64_t orientation) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageOrientation(
    wand->getWand(), (OrientationType)orientation);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image orientation");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImagePage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImagePage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image page");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageProfile,
    const String& name, const String& profile) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageProfile(
    wand->getWand(), name.c_str(), profile.c_str(), profile.length());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image profile");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageProperty,
    const String& name, const String& value) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageProperty(
    wand->getWand(), name.c_str(), value.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image property");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageRedPrimary, double x, double y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageRedPrimary(wand->getWand(), x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image red primary");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageRenderingIntent,
    int64_t rendering_intent) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageRenderingIntent(
    wand->getWand(), (RenderingIntent)rendering_intent);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image rendering intent");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageResolution,
    double x_resolution, double y_resolution) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageResolution(
    wand->getWand(), x_resolution, y_resolution);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image resolution");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageScene, int64_t scene) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageScene(wand->getWand(), scene);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image scene");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageTicksPerSecond,
    int64_t ticks_per_second) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageTicksPerSecond(
    wand->getWand(), ticks_per_second);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image ticks per second");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageType, int64_t image_type) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageType(
    wand->getWand(), (ImageType)image_type);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image type");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageUnits, int64_t units) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageUnits(
    wand->getWand(), (ResolutionType)units);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image units");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setImageVirtualPixelMethod,
    int64_t method) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSetImageVirtualPixelMethod(
    wand->getWand(), (VirtualPixelMethod)method);
  return true;
}

static bool HHVM_METHOD(Imagick, setImageWhitePoint, double x, double y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetImageWhitePoint(wand->getWand(), x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set image white point");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setInterlaceScheme,
    int64_t interlace_scheme) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetInterlaceScheme(
    wand->getWand(), (InterlaceType)interlace_scheme);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set interlace scheme");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setIteratorIndex, int64_t index) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetIteratorIndex(wand->getWand(), index);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set iterator index");
  }
  setImagePending(Object{this_}, false);
  return true;
}

static bool HHVM_METHOD(Imagick, setLastIterator) {
  auto wand = getMagickWandResource(Object{this_});
  MagickSetLastIterator(wand->getWand());
  setImagePending(Object{this_}, true);
  return true;
}

static bool HHVM_METHOD(Imagick, setOption,
    const String& key, const String& value) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetOption(wand->getWand(), key.c_str(), value.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set option");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setPage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetPage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set page");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setPointSize, double point_size) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetPointsize(wand->getWand(), point_size);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set point size");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setResolution,
    double x_resolution, double y_resolution) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetResolution(
    wand->getWand(), x_resolution, y_resolution);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set resolution");
  }
  return true;
}

static bool HHVM_STATIC_METHOD(Imagick, setResourceLimit,
    int64_t type, int64_t limit) {
  auto status = MagickSetResourceLimit((ResourceType)type, limit);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set resource limit");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setSamplingFactors, const Array& factors) {
  auto wand = getMagickWandResource(Object{this_});
  auto array = toDoubleArray(factors);
  if (array.empty()) {
    IMAGICK_THROW("Can't read array");
  }
  auto status = MagickSetSamplingFactors(
    wand->getWand(), array.size(), array.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set sampling factors");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setSize, int64_t columns, int64_t rows) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetSize(wand->getWand(), columns, rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set size");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setSizeOffset,
    int64_t columns, int64_t rows, int64_t offset) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetSizeOffset(wand->getWand(), columns, rows, offset);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set size offset");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, setType, int64_t image_type) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSetType(
    wand->getWand(), (ImageType)image_type);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to set type");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, shadeImage,
    bool gray, double azimuth, double elevation) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickShadeImage(
    wand->getWand(), toMagickBool(gray), azimuth, elevation);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to shade image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, shadowImage,
    double opacity, double sigma, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickShadowImage(wand->getWand(), opacity, sigma, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to shadow image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, sharpenImage,
    double radius, double sigma, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSharpenImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sharpen image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, shaveImage, int64_t columns, int64_t rows) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickShaveImage(wand->getWand(), columns, rows);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to shave image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, shearImage,
    const Variant& background, double x_shear, double y_shear) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(background);
  auto status = MagickShearImage(
    wand->getWand(), pixel->getWand(), x_shear, y_shear);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to shear image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, sigmoidalContrastImage,
    bool sharpen, double alpha, double beta, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSigmoidalContrastImageChannel(
    wand->getWand(), (ChannelType)channel, toMagickBool(sharpen), alpha, beta);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sigmoidal contrast image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, sketchImage,
    double radius, double sigma, double angle) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSketchImage(wand->getWand(), radius, sigma, angle);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sketch image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, solarizeImage, int64_t threshold) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSolarizeImage(wand->getWand(), threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to solarize image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, sparseColorImage,
    int64_t sparse, const Array& arguments, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto array = toDoubleArray(arguments);
  if (array.empty()) {
    IMAGICK_THROW("The map must contain only numeric values");
  }
  auto status = MagickSparseColorImage(
    wand->getWand(), (ChannelType)channel, (SparseColorMethod)sparse,
    array.size(), array.data());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to sparse color image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, spliceImage,
    int64_t width, int64_t height, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSpliceImage(wand->getWand(), width, height, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to splice image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, spreadImage, double radius) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSpreadImage(wand->getWand(), radius);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to spread image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, steganoImage,
    const Object& watermark_wand, int64_t offset) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(watermark_wand);
  auto magick = MagickSteganoImage(wand->getWand(), wand2->getWand(), offset);
  if (magick == nullptr) {
    IMAGICK_THROW("Stegano image failed");
  }
  return createImagick(magick);
}

static Object HHVM_METHOD(Imagick, stereoImage, const Object& offset_wand) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(offset_wand);
  auto magick = MagickStereoImage(wand->getWand(), wand2->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Stereo image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, stripImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickStripImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to strip image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, swirlImage, double degrees) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickSwirlImage(wand->getWand(), degrees);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to swirl image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, textureImage, const Object& texture_wand) {
  auto wand = getMagickWandResource(Object{this_});
  auto wand2 = getMagickWandResource(texture_wand);
  auto magick = MagickTextureImage(wand->getWand(), wand2->getWand());
  if (magick == nullptr) {
    IMAGICK_THROW("Texture image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, thresholdImage,
    double threshold, int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickThresholdImageChannel(
    wand->getWand(), (ChannelType)channel, threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to threshold image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, thumbnailImage,
    int64_t cols, int64_t rows, bool bestfit, bool fill) {
  auto wand = getMagickWandResource(Object{this_});
  auto geometry = ImageGeometry(wand->getWand()).toThumbnail(
      {cols, rows}, bestfit);

  if (geometry.empty()) {
    IMAGICK_THROW("Invalid image geometry");
  }

  // Resize the image to the new size
  auto status = MagickThumbnailImage(
    wand->getWand(), geometry.getWidth(), geometry.getHeight());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to thumbnail image");
  }

  // If the image does not fill the box completely the box is filled with
  // image's background color.
  if (bestfit && fill) {
    auto extent_x = std::max<ssize_t>(0, (cols - geometry.getWidth()) / 2);
    auto extent_y = std::max<ssize_t>(0, (rows - geometry.getHeight()) / 2);
    auto status = MagickExtentImage(
      wand->getWand(), cols, rows, -extent_x, -extent_y);
    if (status == MagickFalse) {
      IMAGICK_THROW("Unable to resize and fill image");
    }
  }

  return true;
}

static bool HHVM_METHOD(Imagick, tintImage,
    const Variant& tint, const Variant& opacity) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(tint);
  auto opacityPixel = buildOpacityWand(opacity);
  auto status = MagickTintImage(
    wand->getWand(), pixel->getWand(), opacityPixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable tint image");
  }
  return true;
}

static Object HHVM_METHOD(Imagick, transformImage,
    const String& crop, const String& geometry) {
  auto wand = getMagickWandResource(Object{this_});
  auto magick = MagickTransformImage(
    wand->getWand(), crop.c_str(), geometry.c_str());
  if (!magick) {
    IMAGICK_THROW("Transforming image failed");
  }
  return createImagick(magick);
}

static bool HHVM_METHOD(Imagick, transparentPaintImage,
    const Variant& target, double alpha, double fuzz, bool invert) {
  auto wand = getMagickWandResource(Object{this_});
  auto targetPixel = buildColorWand(target);
  auto status = MagickTransparentPaintImage(
    wand->getWand(), targetPixel->getWand(),
    alpha, fuzz, toMagickBool(invert));
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to paint transparent image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, transposeImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickTransposeImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to transpose image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, transverseImage) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickTransverseImage(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to transverse image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, trimImage, double fuzz) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickTrimImage(wand->getWand(), fuzz);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to trim image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, uniqueImageColors) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickUniqueImageColors(wand->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to get unique image colors");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, unsharpMaskImage,
    double radius, double sigma, double amount, double threshold,
    int64_t channel) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickUnsharpMaskImageChannel(
    wand->getWand(), (ChannelType)channel, radius, sigma, amount, threshold);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to unsharp mask image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, vignetteImage,
    double blackPoint, double whitePoint, int64_t x, int64_t y) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickVignetteImage(
    wand->getWand(), blackPoint, whitePoint, x, y);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to apply vignette filter");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, waveImage,
    double amplitude, double length) {
  auto wand = getMagickWandResource(Object{this_});
  auto status = MagickWaveImage(wand->getWand(), amplitude, length);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to wave image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, whiteThresholdImage,
    const Variant& threshold) {
  auto wand = getMagickWandResource(Object{this_});
  auto pixel = buildColorWand(threshold);
  auto status = MagickWhiteThresholdImage(wand->getWand(), pixel->getWand());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to white threshold image");
  }
  return true;
}

static bool HHVM_METHOD(Imagick, writeImage, const String& filename) {
  auto wand = getMagickWandResource(Object{this_});
  imagickWriteOp(wand->getWand(), filename, MagickWriteImage);
  return true;
}

static bool HHVM_METHOD(Imagick, writeImageFile,
    const OptResource& filehandle, const String& format) {
  auto wand = getMagickWandResource(Object{this_});
  imagickWriteOp(wand->getWand(), filehandle, format, MagickWriteImageFile);
  return true;
}

static bool HHVM_METHOD(Imagick, writeImages,
    const String& filename, bool adjoin) {
  auto wand = getMagickWandResource(Object{this_});
  imagickWriteOp(wand->getWand(), filename,
    [=](MagickWand* magick, const char* path) {
      return MagickWriteImages(magick, path, toMagickBool(adjoin));
    });
  return true;
}

static bool HHVM_METHOD(Imagick, writeImagesFile,
    const OptResource& filehandle, const String& format) {
  auto wand = getMagickWandResource(Object{this_});
  imagickWriteOp(wand->getWand(), filehandle, format, MagickWriteImagesFile);
  return true;
}

// Countable interface
static int64_t HHVM_METHOD(Imagick, count) {
  return HHVM_MN(Imagick, getNumberImages)(this_);
}

// Iterator interface
static Object HHVM_METHOD(Imagick, current) {
  return Object{this_};
}

static int64_t HHVM_METHOD(Imagick, key) {
  return HHVM_MN(Imagick, getIteratorIndex)(this_);
}

static void HHVM_METHOD(Imagick, next) {
  HHVM_MN(Imagick, nextImage)(this_);
}

static void HHVM_METHOD(Imagick, rewind) {
  HHVM_MN(Imagick, setFirstIterator)(this_);
}

static bool HHVM_METHOD(Imagick, valid) {
  return !getImagePending(Object{this_});
}

template<size_t (*get)(_MagickWand*)>
static Variant get_size(const Object& this_) {
  return get(getMagickWandResource(Object{this_})->getWand());
}

static Variant get_image_format(const Object& this_) {
  return HHVM_MN(Imagick, getImageFormat)(this_.get());
}

static Native::PropAccessor imagick_properties[] = {
  { "width", get_size<MagickGetImageWidth> },
  { "height", get_size<MagickGetImageHeight> },
  { "format", get_image_format },
  { nullptr }
};

Native::PropAccessorMap imagick_properties_map{imagick_properties};
struct ImagickPropHandler : Native::MapPropHandler<ImagickPropHandler> {
  static constexpr Native::PropAccessorMap& map = imagick_properties_map;
};

#undef IMAGICK_THROW

void ImagickExtension::registerNativeImagickClass() {
  HHVM_ME(Imagick, adaptiveBlurImage);
  HHVM_ME(Imagick, adaptiveResizeImage);
  HHVM_ME(Imagick, adaptiveSharpenImage);
  HHVM_ME(Imagick, adaptiveThresholdImage);
  HHVM_ME(Imagick, addImage);
  HHVM_ME(Imagick, addNoiseImage);
  HHVM_ME(Imagick, affineTransformImage);
  HHVM_ME(Imagick, animateImages);
  HHVM_ME(Imagick, annotateImage);
  HHVM_ME(Imagick, appendImages);
  HHVM_ME(Imagick, averageImages);
  HHVM_ME(Imagick, blackThresholdImage);
  HHVM_ME(Imagick, blurImage);
  HHVM_ME(Imagick, borderImage);
  HHVM_ME(Imagick, charcoalImage);
  HHVM_ME(Imagick, chopImage);
  HHVM_ME(Imagick, clear);
  HHVM_ME(Imagick, clipImage);
  HHVM_ME(Imagick, clipPathImage);
  HHVM_ME(Imagick, __clone);
  HHVM_ME(Imagick, clutImage);
  HHVM_ME(Imagick, coalesceImages);
  HHVM_ME(Imagick, colorFloodfillImage);
  HHVM_ME(Imagick, colorizeImage);
  HHVM_ME(Imagick, combineImages);
  HHVM_ME(Imagick, commentImage);
  HHVM_ME(Imagick, compareImageChannels);
  HHVM_ME(Imagick, compareImageLayers);
  HHVM_ME(Imagick, compareImages);
  HHVM_ME(Imagick, compositeImage);
  HHVM_ME(Imagick, __construct);
  HHVM_ME(Imagick, contrastImage);
  HHVM_ME(Imagick, contrastStretchImage);
  HHVM_ME(Imagick, convolveImage);
  HHVM_ME(Imagick, cropImage);
  HHVM_ME(Imagick, cropThumbnailImage);
  HHVM_ME(Imagick, cycleColormapImage);
  HHVM_ME(Imagick, decipherImage);
  HHVM_ME(Imagick, deconstructImages);
  HHVM_ME(Imagick, deleteImageArtifact);
  HHVM_ME(Imagick, deskewImage);
  HHVM_ME(Imagick, despeckleImage);
  HHVM_ME(Imagick, destroy);
  HHVM_ME(Imagick, displayImage);
  HHVM_ME(Imagick, displayImages);
  HHVM_ME(Imagick, distortImage);
  HHVM_ME(Imagick, drawImage);
  HHVM_ME(Imagick, edgeImage);
  HHVM_ME(Imagick, embossImage);
  HHVM_ME(Imagick, encipherImage);
  HHVM_ME(Imagick, enhanceImage);
  HHVM_ME(Imagick, equalizeImage);
  HHVM_ME(Imagick, evaluateImage);
  HHVM_ME(Imagick, exportImagePixels);
  HHVM_ME(Imagick, extentImage);
  HHVM_ME(Imagick, flattenImages);
  HHVM_ME(Imagick, flipImage);
  HHVM_ME(Imagick, floodFillPaintImage);
  HHVM_ME(Imagick, flopImage);
  HHVM_ME(Imagick, frameImage);
  HHVM_ME(Imagick, functionImage);
  HHVM_ME(Imagick, fxImage);
  HHVM_ME(Imagick, gammaImage);
  HHVM_ME(Imagick, gaussianBlurImage);
  HHVM_ME(Imagick, getColorspace);
  HHVM_ME(Imagick, getCompression);
  HHVM_ME(Imagick, getCompressionQuality);
  HHVM_STATIC_ME(Imagick, getCopyright);
  HHVM_ME(Imagick, getFilename);
  HHVM_ME(Imagick, getFont);
  HHVM_ME(Imagick, getFormat);
  HHVM_ME(Imagick, getGravity);
  HHVM_STATIC_ME(Imagick, getHomeURL);
  HHVM_ME(Imagick, getImage);
  HHVM_ME(Imagick, getImageAlphaChannel);
  HHVM_ME(Imagick, getImageArtifact);
  HHVM_ME(Imagick, getImageBackgroundColor);
  HHVM_ME(Imagick, getImageBlob);
  HHVM_ME(Imagick, getImageBluePrimary);
  HHVM_ME(Imagick, getImageBorderColor);
  HHVM_ME(Imagick, getImageChannelDepth);
  HHVM_ME(Imagick, getImageChannelDistortion);
  HHVM_ME(Imagick, getImageChannelDistortions);
  HHVM_ME(Imagick, getImageChannelExtrema);
  HHVM_ME(Imagick, getImageChannelKurtosis);
  HHVM_ME(Imagick, getImageChannelMean);
  HHVM_ME(Imagick, getImageChannelRange);
  HHVM_ME(Imagick, getImageChannelStatistics);
  HHVM_ME(Imagick, getImageClipMask);
  HHVM_ME(Imagick, getImageColormapColor);
  HHVM_ME(Imagick, getImageColors);
  HHVM_ME(Imagick, getImageColorspace);
  HHVM_ME(Imagick, getImageCompose);
  HHVM_ME(Imagick, getImageCompression);
  HHVM_ME(Imagick, getImageDelay);
  HHVM_ME(Imagick, getImageDepth);
  HHVM_ME(Imagick, getImageDispose);
  HHVM_ME(Imagick, getImageDistortion);
  HHVM_ME(Imagick, getImageExtrema);
  HHVM_ME(Imagick, getImageFilename);
  HHVM_ME(Imagick, getImageFormat);
  HHVM_ME(Imagick, getImageGamma);
  HHVM_ME(Imagick, getImageGeometry);
  HHVM_ME(Imagick, getImageGravity);
  HHVM_ME(Imagick, getImageGreenPrimary);
  HHVM_ME(Imagick, getImageHeight);
  HHVM_ME(Imagick, getImageHistogram);
  HHVM_ME(Imagick, getImageIndex);
  HHVM_ME(Imagick, getImageInterlaceScheme);
  HHVM_ME(Imagick, getImageInterpolateMethod);
  HHVM_ME(Imagick, getImageIterations);
  HHVM_ME(Imagick, getImageLength);
  HHVM_ME(Imagick, getImageMatte);
  HHVM_ME(Imagick, getImageMatteColor);
  HHVM_ME(Imagick, getImageMimeType);
  HHVM_ME(Imagick, getImageOrientation);
  HHVM_ME(Imagick, getImagePage);
  HHVM_ME(Imagick, getImagePixelColor);
  HHVM_ME(Imagick, getImageProfile);
  HHVM_ME(Imagick, getImageProfiles);
  HHVM_ME(Imagick, getImageProperties);
  HHVM_ME(Imagick, getImageProperty);
  HHVM_ME(Imagick, getImageRedPrimary);
  HHVM_ME(Imagick, getImageRegion);
  HHVM_ME(Imagick, getImageRenderingIntent);
  HHVM_ME(Imagick, getImageResolution);
  HHVM_ME(Imagick, getImagesBlob);
  HHVM_ME(Imagick, getImageScene);
  HHVM_ME(Imagick, getImageSignature);
  HHVM_ME(Imagick, getImageSize);
  HHVM_ME(Imagick, getImageTicksPerSecond);
  HHVM_ME(Imagick, getImageTotalInkDensity);
  HHVM_ME(Imagick, getImageType);
  HHVM_ME(Imagick, getImageUnits);
  HHVM_ME(Imagick, getImageVirtualPixelMethod);
  HHVM_ME(Imagick, getImageWhitePoint);
  HHVM_ME(Imagick, getImageWidth);
  HHVM_ME(Imagick, getInterlaceScheme);
  HHVM_ME(Imagick, getIteratorIndex);
  HHVM_ME(Imagick, getNumberImages);
  HHVM_ME(Imagick, getOption);
  HHVM_STATIC_ME(Imagick, getPackageName);
  HHVM_ME(Imagick, getPage);
  HHVM_ME(Imagick, getPixelIterator);
  HHVM_ME(Imagick, getPixelRegionIterator);
  HHVM_ME(Imagick, getPointSize);
  HHVM_STATIC_ME(Imagick, getQuantumDepth);
  HHVM_STATIC_ME(Imagick, getQuantumRange);
  HHVM_STATIC_ME(Imagick, getReleaseDate);
  HHVM_STATIC_ME(Imagick, getResource);
  HHVM_STATIC_ME(Imagick, getResourceLimit);
  HHVM_ME(Imagick, getSamplingFactors);
  HHVM_ME(Imagick, getSize);
  HHVM_ME(Imagick, getSizeOffset);
  HHVM_STATIC_ME(Imagick, getVersion);
  HHVM_ME(Imagick, haldClutImage);
  HHVM_ME(Imagick, hasNextImage);
  HHVM_ME(Imagick, hasPreviousImage);
  HHVM_ME(Imagick, identifyImage);
  HHVM_ME(Imagick, implodeImage);
  HHVM_ME(Imagick, importImagePixels);
  HHVM_ME(Imagick, labelImage);
  HHVM_ME(Imagick, levelImage);
  HHVM_ME(Imagick, linearStretchImage);
  HHVM_ME(Imagick, liquidRescaleImage);
  HHVM_ME(Imagick, magnifyImage);
  HHVM_ME(Imagick, mapImage);
  HHVM_ME(Imagick, matteFloodfillImage);
  HHVM_ME(Imagick, medianFilterImage);
  HHVM_ME(Imagick, mergeImageLayers);
  HHVM_ME(Imagick, minifyImage);
  HHVM_ME(Imagick, modulateImage);
  HHVM_ME(Imagick, montageImage);
  HHVM_ME(Imagick, morphImages);
  HHVM_ME(Imagick, mosaicImages);
  HHVM_ME(Imagick, motionBlurImage);
  HHVM_ME(Imagick, negateImage);
  HHVM_ME(Imagick, newImage);
  HHVM_ME(Imagick, newPseudoImage);
  HHVM_ME(Imagick, nextImage);
  HHVM_ME(Imagick, normalizeImage);
  HHVM_ME(Imagick, oilPaintImage);
  HHVM_ME(Imagick, opaquePaintImage);
  HHVM_ME(Imagick, optimizeImageLayers);
  HHVM_ME(Imagick, orderedPosterizeImage);
  HHVM_ME(Imagick, paintFloodfillImage);
  HHVM_ME(Imagick, paintOpaqueImage);
  HHVM_ME(Imagick, paintTransparentImage);
  HHVM_ME(Imagick, pingImage);
  HHVM_ME(Imagick, pingImageBlob);
  HHVM_ME(Imagick, pingImageFile);
  HHVM_ME(Imagick, polaroidImage);
  HHVM_ME(Imagick, posterizeImage);
  HHVM_ME(Imagick, previewImages);
  HHVM_ME(Imagick, previousImage);
  HHVM_ME(Imagick, profileImage);
  HHVM_ME(Imagick, quantizeImage);
  HHVM_ME(Imagick, quantizeImages);
  HHVM_ME(Imagick, queryFontMetrics);
  HHVM_STATIC_ME(Imagick, queryFonts);
  HHVM_STATIC_ME(Imagick, queryFormats);
  HHVM_ME(Imagick, radialBlurImage);
  HHVM_ME(Imagick, raiseImage);
  HHVM_ME(Imagick, randomThresholdImage);
  HHVM_ME(Imagick, readImage);
  HHVM_ME(Imagick, readImageBlob);
  HHVM_ME(Imagick, readImageFile);
  HHVM_ME(Imagick, readImages);
  HHVM_ME(Imagick, recolorImage);
  HHVM_ME(Imagick, reduceNoiseImage);
  HHVM_ME(Imagick, remapImage);
  HHVM_ME(Imagick, removeImage);
  HHVM_ME(Imagick, removeImageProfile);
  HHVM_ME(Imagick, resampleImage);
  HHVM_ME(Imagick, resetImagePage);
  HHVM_ME(Imagick, resizeImage);
  HHVM_ME(Imagick, rollImage);
  HHVM_ME(Imagick, rotateImage);
  HHVM_ME(Imagick, roundCorners);
  HHVM_ME(Imagick, roundCornersImage);
  HHVM_ME(Imagick, sampleImage);
  HHVM_ME(Imagick, scaleImage);
  HHVM_ME(Imagick, segmentImage);
  HHVM_ME(Imagick, separateImageChannel);
  HHVM_ME(Imagick, sepiaToneImage);
  HHVM_ME(Imagick, setBackgroundColor);
  HHVM_ME(Imagick, setColorspace);
  HHVM_ME(Imagick, setCompression);
  HHVM_ME(Imagick, setCompressionQuality);
  HHVM_ME(Imagick, setFilename);
  HHVM_ME(Imagick, setFirstIterator);
  HHVM_ME(Imagick, setFont);
  HHVM_ME(Imagick, setFormat);
  HHVM_ME(Imagick, setGravity);
  HHVM_ME(Imagick, setImage);
  HHVM_ME(Imagick, setImageAlphaChannel);
  HHVM_ME(Imagick, setImageArtifact);
  HHVM_ME(Imagick, setImageBackgroundColor);
  HHVM_ME(Imagick, setImageBias);
  HHVM_ME(Imagick, setImageBluePrimary);
  HHVM_ME(Imagick, setImageBorderColor);
  HHVM_ME(Imagick, setImageChannelDepth);
  HHVM_ME(Imagick, setImageClipMask);
  HHVM_ME(Imagick, setImageColormapColor);
  HHVM_ME(Imagick, setImageColorspace);
  HHVM_ME(Imagick, setImageCompose);
  HHVM_ME(Imagick, setImageCompression);
  HHVM_ME(Imagick, setImageCompressionQuality);
  HHVM_ME(Imagick, setImageDelay);
  HHVM_ME(Imagick, setImageDepth);
  HHVM_ME(Imagick, setImageDispose);
  HHVM_ME(Imagick, setImageExtent);
  HHVM_ME(Imagick, setImageFilename);
  HHVM_ME(Imagick, setImageFormat);
  HHVM_ME(Imagick, setImageGamma);
  HHVM_ME(Imagick, setImageGravity);
  HHVM_ME(Imagick, setImageGreenPrimary);
  HHVM_ME(Imagick, setImageIndex);
  HHVM_ME(Imagick, setImageInterlaceScheme);
  HHVM_ME(Imagick, setImageInterpolateMethod);
  HHVM_ME(Imagick, setImageIterations);
  HHVM_ME(Imagick, setImageMatte);
  HHVM_ME(Imagick, setImageMatteColor);
  HHVM_ME(Imagick, setImageOpacity);
  HHVM_ME(Imagick, setImageOrientation);
  HHVM_ME(Imagick, setImagePage);
  HHVM_ME(Imagick, setImageProfile);
  HHVM_ME(Imagick, setImageProperty);
  HHVM_ME(Imagick, setImageRedPrimary);
  HHVM_ME(Imagick, setImageRenderingIntent);
  HHVM_ME(Imagick, setImageResolution);
  HHVM_ME(Imagick, setImageScene);
  HHVM_ME(Imagick, setImageTicksPerSecond);
  HHVM_ME(Imagick, setImageType);
  HHVM_ME(Imagick, setImageUnits);
  HHVM_ME(Imagick, setImageVirtualPixelMethod);
  HHVM_ME(Imagick, setImageWhitePoint);
  HHVM_ME(Imagick, setInterlaceScheme);
  HHVM_ME(Imagick, setIteratorIndex);
  HHVM_ME(Imagick, setLastIterator);
  HHVM_ME(Imagick, setOption);
  HHVM_ME(Imagick, setPage);
  HHVM_ME(Imagick, setPointSize);
  HHVM_ME(Imagick, setResolution);
  HHVM_STATIC_ME(Imagick, setResourceLimit);
  HHVM_ME(Imagick, setSamplingFactors);
  HHVM_ME(Imagick, setSize);
  HHVM_ME(Imagick, setSizeOffset);
  HHVM_ME(Imagick, setType);
  HHVM_ME(Imagick, shadeImage);
  HHVM_ME(Imagick, shadowImage);
  HHVM_ME(Imagick, sharpenImage);
  HHVM_ME(Imagick, shaveImage);
  HHVM_ME(Imagick, shearImage);
  HHVM_ME(Imagick, sigmoidalContrastImage);
  HHVM_ME(Imagick, sketchImage);
  HHVM_ME(Imagick, solarizeImage);
  HHVM_ME(Imagick, sparseColorImage);
  HHVM_ME(Imagick, spliceImage);
  HHVM_ME(Imagick, spreadImage);
  HHVM_ME(Imagick, steganoImage);
  HHVM_ME(Imagick, stereoImage);
  HHVM_ME(Imagick, stripImage);
  HHVM_ME(Imagick, swirlImage);
  HHVM_ME(Imagick, textureImage);
  HHVM_ME(Imagick, thresholdImage);
  HHVM_ME(Imagick, thumbnailImage);
  HHVM_ME(Imagick, tintImage);
  HHVM_ME(Imagick, transformImage);
  HHVM_ME(Imagick, transparentPaintImage);
  HHVM_ME(Imagick, transposeImage);
  HHVM_ME(Imagick, transverseImage);
  HHVM_ME(Imagick, trimImage);
  HHVM_ME(Imagick, uniqueImageColors);
  HHVM_ME(Imagick, unsharpMaskImage);
  HHVM_ME(Imagick, vignetteImage);
  HHVM_ME(Imagick, waveImage);
  HHVM_ME(Imagick, whiteThresholdImage);
  HHVM_ME(Imagick, writeImage);
  HHVM_ME(Imagick, writeImageFile);
  HHVM_ME(Imagick, writeImages);
  HHVM_ME(Imagick, writeImagesFile);
  // Countable interface
  HHVM_ME(Imagick, count);
  // Iterator interface
  HHVM_ME(Imagick, current);
  HHVM_ME(Imagick, key);
  HHVM_ME(Imagick, next);
  HHVM_ME(Imagick, rewind);
  HHVM_ME(Imagick, valid);

  Native::registerNativePropHandler<ImagickPropHandler>(s_Imagick);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
