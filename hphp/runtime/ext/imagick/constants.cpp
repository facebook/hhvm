/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/imagick/constants.h"
#include "hphp/runtime/ext/imagick/ext_imagick.h"

namespace HPHP {

const StaticString
  // coord
  s_x("x"),
  s_y("y"),
  // size
  s_columns("columns"),
  s_rows("rows"),
  // geometry
  s_width("width"),
  s_height("height"),
  // affine
  s_sx("sx"),
  s_rx("rx"),
  s_ry("ry"),
  s_sy("sy"),
  s_tx("tx"),
  s_ty("ty"),
  // color
  s_r("r"),
  s_g("g"),
  s_b("b"),
  s_a("a"),
  s_hue("hue"),
  s_saturation("saturation"),
  s_luminosity("luminosity"),
  // extrema
  s_min("min"),
  s_max("max"),
  // channel extrema/range
  s_minima("minima"),
  s_maxima("maxima"),
  // channel kurtosis
  s_kurtosis("kurtosis"),
  s_skewness("skewness"),
  // channel mean
  s_mean("mean"),
  s_standardDeviation("standardDeviation"),
  // other channel statistics
  s_depth("depth"),
  // quantum depth
  s_quantumDepthLong("quantumDepthLong"),
  s_quantumDepthString("quantumDepthString"),
  // quantum range
  s_quantumRangeLong("quantumRangeLong"),
  s_quantumRangeString("quantumRangeString"),
  // version
  s_versionNumber("versionNumber"),
  s_versionString("versionString"),
  // identify
  s_imageName("imageName"),
  s_mimetype("mimetype"),
  s_geometry("geometry"),
  s_resolution("resolution"),
  s_signature("signature"),
  s_rawOutput("rawOutput"),
  s_format("format"),
  s_units("units"),
  s_type("type"),
  s_colorSpace("colorSpace"),
  s_fileSize("fileSize"),
  s_compression("compression"),
  // font metrics
  s_characterWidth("characterWidth"),
  s_characterHeight("characterHeight"),
  s_ascender("ascender"),
  s_descender("descender"),
  s_textWidth("textWidth"),
  s_textHeight("textHeight"),
  s_maxHorizontalAdvance("maxHorizontalAdvance"),
  s_x1("x1"),
  s_y1("y1"),
  s_x2("x2"),
  s_y2("y2"),
  s_originX("originX"),
  s_originY("originY"),
  s_boundingBox("boundingBox"),
  // class name
  s_Imagick("Imagick"),
  s_ImagickDraw("ImagickDraw"),
  s_ImagickPixel("ImagickPixel"),
  s_ImagickPixelIterator("ImagickPixelIterator");

void loadImagickConstants() {
  // COLOR_* constants
  // Colortype constants. These constants are mainly used with ImagickPixel.
  HHVM_RCC_I(Imagick, COLOR_BLACK, IMAGICK_COLOR_BLACK);
  HHVM_RCC_I(Imagick, COLOR_BLUE, IMAGICK_COLOR_BLUE);
  HHVM_RCC_I(Imagick, COLOR_CYAN, IMAGICK_COLOR_CYAN);
  HHVM_RCC_I(Imagick, COLOR_GREEN, IMAGICK_COLOR_GREEN);
  HHVM_RCC_I(Imagick, COLOR_RED, IMAGICK_COLOR_RED);
  HHVM_RCC_I(Imagick, COLOR_YELLOW, IMAGICK_COLOR_YELLOW);
  HHVM_RCC_I(Imagick, COLOR_MAGENTA, IMAGICK_COLOR_MAGENTA);
  HHVM_RCC_I(Imagick, COLOR_OPACITY, IMAGICK_COLOR_OPACITY);
  HHVM_RCC_I(Imagick, COLOR_ALPHA, IMAGICK_COLOR_ALPHA);
  HHVM_RCC_I(Imagick, COLOR_FUZZ, IMAGICK_COLOR_FUZZ);

  // DISPOSE constants
  // Dispose type constants
  HHVM_RCC_I(Imagick, DISPOSE_UNRECOGNIZED, UnrecognizedDispose);
  HHVM_RCC_I(Imagick, DISPOSE_UNDEFINED, UndefinedDispose);
  HHVM_RCC_I(Imagick, DISPOSE_NONE, NoneDispose);
  HHVM_RCC_I(Imagick, DISPOSE_BACKGROUND, BackgroundDispose);
  HHVM_RCC_I(Imagick, DISPOSE_PREVIOUS, PreviousDispose);

  // Composite Operator Constants
  HHVM_RCC_I(Imagick, COMPOSITE_DEFAULT, OverCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_UNDEFINED, UndefinedCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_NO, NoCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_ADD, AddCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_ATOP, AtopCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_BLEND, BlendCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_BUMPMAP, BumpmapCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_CLEAR, ClearCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COLORBURN, ColorBurnCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COLORDODGE, ColorDodgeCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COLORIZE, ColorizeCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYBLACK, CopyBlackCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYBLUE, CopyBlueCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPY, CopyCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYCYAN, CopyCyanCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYGREEN, CopyGreenCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYMAGENTA, CopyMagentaCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYOPACITY, CopyOpacityCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYRED, CopyRedCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_COPYYELLOW, CopyYellowCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DARKEN, DarkenCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DSTATOP, DstAtopCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DST, DstCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DSTIN, DstInCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DSTOUT, DstOutCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DSTOVER, DstOverCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DIFFERENCE, DifferenceCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DISPLACE, DisplaceCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DISSOLVE, DissolveCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_EXCLUSION, ExclusionCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_HARDLIGHT, HardLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_HUE, HueCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_IN, InCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LIGHTEN, LightenCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LUMINIZE, LuminizeCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MINUS, MinusCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MODULATE, ModulateCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MULTIPLY, MultiplyCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_OUT, OutCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_OVER, OverCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_OVERLAY, OverlayCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_PLUS, PlusCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_REPLACE, ReplaceCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SATURATE, SaturateCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SCREEN, ScreenCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SOFTLIGHT, SoftLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SRCATOP, SrcAtopCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SRC, SrcCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SRCIN, SrcInCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SRCOUT, SrcOutCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SRCOVER, SrcOverCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_SUBTRACT, SubtractCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_THRESHOLD, ThresholdCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_XOR, XorCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_CHANGEMASK, ChangeMaskCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LINEARLIGHT, LinearLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DIVIDE, DivideCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DISTORT, DistortCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_BLUR, BlurCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_PEGTOPLIGHT, PegtopLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_VIVIDLIGHT, VividLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_PINLIGHT, PinLightCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LINEARDODGE, LinearDodgeCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LINEARBURN, LinearBurnCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MATHEMATICS, MathematicsCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MODULUSADD, ModulusAddCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MODULUSSUBTRACT, ModulusSubtractCompositeOp);
#if MagickLibVersion >= 0x670
  HHVM_RCC_I(Imagick, COMPOSITE_MINUSDST, MinusDstCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DIVIDEDST, DivideDstCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DIVIDESRC, DivideSrcCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_MINUSSRC, MinusSrcCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_DARKENINTENSITY, DarkenIntensityCompositeOp);
  HHVM_RCC_I(Imagick, COMPOSITE_LIGHTENINTENSITY, LightenIntensityCompositeOp);
#endif

  // MONTAGEMODE constants
  HHVM_RCC_I(Imagick, MONTAGEMODE_FRAME, FrameMode);
  HHVM_RCC_I(Imagick, MONTAGEMODE_UNFRAME, UnframeMode);
  HHVM_RCC_I(Imagick, MONTAGEMODE_CONCATENATE, ConcatenateMode);

  // STYLE constants
  HHVM_RCC_I(Imagick, STYLE_NORMAL, NormalStyle);
  HHVM_RCC_I(Imagick, STYLE_ITALIC, ItalicStyle);
  HHVM_RCC_I(Imagick, STYLE_OBLIQUE, ObliqueStyle);
  HHVM_RCC_I(Imagick, STYLE_ANY, AnyStyle);

  // FILTER constants
  HHVM_RCC_I(Imagick, FILTER_UNDEFINED, UndefinedFilter);
  HHVM_RCC_I(Imagick, FILTER_POINT, PointFilter);
  HHVM_RCC_I(Imagick, FILTER_BOX, BoxFilter);
  HHVM_RCC_I(Imagick, FILTER_TRIANGLE, TriangleFilter);
  HHVM_RCC_I(Imagick, FILTER_HERMITE, HermiteFilter);
  HHVM_RCC_I(Imagick, FILTER_HANNING, HanningFilter);
  HHVM_RCC_I(Imagick, FILTER_HAMMING, HammingFilter);
  HHVM_RCC_I(Imagick, FILTER_BLACKMAN, BlackmanFilter);
  HHVM_RCC_I(Imagick, FILTER_GAUSSIAN, GaussianFilter);
  HHVM_RCC_I(Imagick, FILTER_QUADRATIC, QuadraticFilter);
  HHVM_RCC_I(Imagick, FILTER_CUBIC, CubicFilter);
  HHVM_RCC_I(Imagick, FILTER_CATROM, CatromFilter);
  HHVM_RCC_I(Imagick, FILTER_MITCHELL, MitchellFilter);
  HHVM_RCC_I(Imagick, FILTER_LANCZOS, LanczosFilter);
  HHVM_RCC_I(Imagick, FILTER_BESSEL, BesselFilter);
  HHVM_RCC_I(Imagick, FILTER_SINC, SincFilter);
  HHVM_RCC_I(Imagick, FILTER_KAISER, KaiserFilter);
  HHVM_RCC_I(Imagick, FILTER_WELSH, WelshFilter);
  HHVM_RCC_I(Imagick, FILTER_PARZEN, ParzenFilter);
  HHVM_RCC_I(Imagick, FILTER_LAGRANGE, LagrangeFilter);
  HHVM_RCC_I(Imagick, FILTER_SENTINEL, SentinelFilter);
  HHVM_RCC_I(Imagick, FILTER_BOHMAN, BohmanFilter);
  HHVM_RCC_I(Imagick, FILTER_BARTLETT, BartlettFilter);
  HHVM_RCC_I(Imagick, FILTER_JINC, JincFilter);
  HHVM_RCC_I(Imagick, FILTER_SINCFAST, SincFastFilter);
  HHVM_RCC_I(Imagick, FILTER_ROBIDOUX, RobidouxFilter);
  HHVM_RCC_I(Imagick, FILTER_LANCZOSSHARP, LanczosSharpFilter);
  HHVM_RCC_I(Imagick, FILTER_LANCZOS2, Lanczos2Filter);
  HHVM_RCC_I(Imagick, FILTER_LANCZOS2SHARP, Lanczos2SharpFilter);
#if MagickLibVersion >= 0x677
  HHVM_RCC_I(Imagick, FILTER_ROBIDOUXSHARP, RobidouxSharpFilter);
  HHVM_RCC_I(Imagick, FILTER_COSINE, CosineFilter);
#endif
#if MagickLibVersion >= 0x678
  HHVM_RCC_I(Imagick, FILTER_SPLINE, SplineFilter);
#endif
#if MagickLibVersion >= 0x681
  HHVM_RCC_I(Imagick, FILTER_LANCZOSRADIUS, LanczosRadiusFilter);
#endif

  // IMGTYPE constants
  HHVM_RCC_I(Imagick, IMGTYPE_UNDEFINED, UndefinedType);
  HHVM_RCC_I(Imagick, IMGTYPE_BILEVEL, BilevelType);
  HHVM_RCC_I(Imagick, IMGTYPE_GRAYSCALE, GrayscaleType);
  HHVM_RCC_I(Imagick, IMGTYPE_GRAYSCALEMATTE, GrayscaleMatteType);
  HHVM_RCC_I(Imagick, IMGTYPE_PALETTEMATTE, PaletteMatteType);
  HHVM_RCC_I(Imagick, IMGTYPE_TRUECOLOR, TrueColorType);
  HHVM_RCC_I(Imagick, IMGTYPE_TRUECOLORMATTE, TrueColorMatteType);
  HHVM_RCC_I(Imagick, IMGTYPE_COLORSEPARATION, ColorSeparationType);
  HHVM_RCC_I(Imagick, IMGTYPE_COLORSEPARATIONMATTE, ColorSeparationMatteType);
  HHVM_RCC_I(Imagick, IMGTYPE_OPTIMIZE, OptimizeType);
  HHVM_RCC_I(Imagick, IMGTYPE_PALETTEBILEVELMATTE, PaletteBilevelMatteType);
  HHVM_RCC_I(Imagick, IMGTYPE_PALETTE, PaletteType);

  // RESOLUTION constants
  HHVM_RCC_I(Imagick, RESOLUTION_UNDEFINED, UndefinedResolution);
  HHVM_RCC_I(Imagick, RESOLUTION_PIXELSPERINCH, PixelsPerInchResolution);
  HHVM_RCC_I(Imagick, RESOLUTION_PIXELSPERCENTIMETER,
                           PixelsPerCentimeterResolution);

  // COMPRESSION constants
  HHVM_RCC_I(Imagick, COMPRESSION_UNDEFINED, UndefinedCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_NO, NoCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_BZIP, BZipCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_FAX, FaxCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_GROUP4, Group4Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_JPEG, JPEGCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_JPEG2000, JPEG2000Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_LOSSLESSJPEG, LosslessJPEGCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_LZW, LZWCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_RLE, RLECompression);
  HHVM_RCC_I(Imagick, COMPRESSION_ZIP, ZipCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_DXT1, DXT1Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_DXT3, DXT3Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_DXT5, DXT5Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_ZIPS, ZipSCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_PIZ, PizCompression);
  HHVM_RCC_I(Imagick, COMPRESSION_PXR24, Pxr24Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_B44, B44Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_B44A, B44ACompression);
  HHVM_RCC_I(Imagick, COMPRESSION_LZMA, LZMACompression);
#if MagickLibVersion >= 0x670
  HHVM_RCC_I(Imagick, COMPRESSION_JBIG1, JBIG1Compression);
  HHVM_RCC_I(Imagick, COMPRESSION_JBIG2, JBIG2Compression);
#endif

  // PAINT constants
  HHVM_RCC_I(Imagick, PAINT_POINT, PointMethod);
  HHVM_RCC_I(Imagick, PAINT_REPLACE, ReplaceMethod);
  HHVM_RCC_I(Imagick, PAINT_FLOODFILL, FloodfillMethod);
  HHVM_RCC_I(Imagick, PAINT_FILLTOBORDER, FillToBorderMethod);
  HHVM_RCC_I(Imagick, PAINT_RESET, ResetMethod);

  // GRAVITY constants
  HHVM_RCC_I(Imagick, GRAVITY_NORTHWEST, NorthWestGravity);
  HHVM_RCC_I(Imagick, GRAVITY_NORTH, NorthGravity);
  HHVM_RCC_I(Imagick, GRAVITY_NORTHEAST, NorthEastGravity);
  HHVM_RCC_I(Imagick, GRAVITY_WEST, WestGravity);
  HHVM_RCC_I(Imagick, GRAVITY_CENTER, CenterGravity);
  HHVM_RCC_I(Imagick, GRAVITY_EAST, EastGravity);
  HHVM_RCC_I(Imagick, GRAVITY_SOUTHWEST, SouthWestGravity);
  HHVM_RCC_I(Imagick, GRAVITY_SOUTH, SouthGravity);
  HHVM_RCC_I(Imagick, GRAVITY_SOUTHEAST, SouthEastGravity);

  // STRETCH constants
  HHVM_RCC_I(Imagick, STRETCH_NORMAL, NormalStretch);
  HHVM_RCC_I(Imagick, STRETCH_ULTRACONDENSED, UltraCondensedStretch);
  HHVM_RCC_I(Imagick, STRETCH_CONDENSED, CondensedStretch);
  HHVM_RCC_I(Imagick, STRETCH_SEMICONDENSED, SemiCondensedStretch);
  HHVM_RCC_I(Imagick, STRETCH_SEMIEXPANDED, SemiExpandedStretch);
  HHVM_RCC_I(Imagick, STRETCH_EXPANDED, ExpandedStretch);
  HHVM_RCC_I(Imagick, STRETCH_EXTRAEXPANDED, ExtraExpandedStretch);
  HHVM_RCC_I(Imagick, STRETCH_ULTRAEXPANDED, UltraExpandedStretch);
  HHVM_RCC_I(Imagick, STRETCH_ANY, AnyStretch);

  // ALIGN constants
  HHVM_RCC_I(Imagick, ALIGN_UNDEFINED, UndefinedAlign);
  HHVM_RCC_I(Imagick, ALIGN_LEFT, LeftAlign);
  HHVM_RCC_I(Imagick, ALIGN_CENTER, CenterAlign);
  HHVM_RCC_I(Imagick, ALIGN_RIGHT, RightAlign);

  // DECORATION constants
  HHVM_RCC_I(Imagick, DECORATION_NO, NoDecoration);
  HHVM_RCC_I(Imagick, DECORATION_UNDERLINE, UnderlineDecoration);
  HHVM_RCC_I(Imagick, DECORATION_OVERLINE, OverlineDecoration);
  HHVM_RCC_I(Imagick, DECORATION_LINETROUGH, LineThroughDecoration);

  // NOISE constants
  HHVM_RCC_I(Imagick, NOISE_UNIFORM, UniformNoise);
  HHVM_RCC_I(Imagick, NOISE_GAUSSIAN, GaussianNoise);
  HHVM_RCC_I(Imagick, NOISE_MULTIPLICATIVEGAUSSIAN,
                           MultiplicativeGaussianNoise);
  HHVM_RCC_I(Imagick, NOISE_IMPULSE, ImpulseNoise);
  HHVM_RCC_I(Imagick, NOISE_LAPLACIAN, LaplacianNoise);
  HHVM_RCC_I(Imagick, NOISE_POISSON, PoissonNoise);
  HHVM_RCC_I(Imagick, NOISE_RANDOM, RandomNoise);

  // CHANNEL constants
  HHVM_RCC_I(Imagick, CHANNEL_UNDEFINED, UndefinedChannel);
  HHVM_RCC_I(Imagick, CHANNEL_RED, RedChannel);
  HHVM_RCC_I(Imagick, CHANNEL_GRAY, GrayChannel);
  HHVM_RCC_I(Imagick, CHANNEL_CYAN, CyanChannel);
  HHVM_RCC_I(Imagick, CHANNEL_GREEN, GreenChannel);
  HHVM_RCC_I(Imagick, CHANNEL_MAGENTA, MagentaChannel);
  HHVM_RCC_I(Imagick, CHANNEL_BLUE, BlueChannel);
  HHVM_RCC_I(Imagick, CHANNEL_YELLOW, YellowChannel);
  HHVM_RCC_I(Imagick, CHANNEL_ALPHA, AlphaChannel);
  HHVM_RCC_I(Imagick, CHANNEL_OPACITY, OpacityChannel);
  HHVM_RCC_I(Imagick, CHANNEL_MATTE, MatteChannel);
  HHVM_RCC_I(Imagick, CHANNEL_BLACK, BlackChannel);
  HHVM_RCC_I(Imagick, CHANNEL_INDEX, IndexChannel);
  HHVM_RCC_I(Imagick, CHANNEL_ALL, AllChannels);
  HHVM_RCC_I(Imagick, CHANNEL_DEFAULT, DefaultChannels);
  HHVM_RCC_I(Imagick, CHANNEL_TRUEALPHA, TrueAlphaChannel);
  HHVM_RCC_I(Imagick, CHANNEL_RGBS, RGBChannels);
  HHVM_RCC_I(Imagick, CHANNEL_SYNC, SyncChannels);
#if MagickLibVersion >= 0x670
  HHVM_RCC_I(Imagick, CHANNEL_COMPOSITES, CompositeChannels);
#endif

  // METRIC constants
  HHVM_RCC_I(Imagick, METRIC_UNDEFINED, UndefinedMetric);
  HHVM_RCC_I(Imagick, METRIC_MEANABSOLUTEERROR, MeanAbsoluteErrorMetric);
  HHVM_RCC_I(Imagick, METRIC_MEANSQUAREERROR, MeanSquaredErrorMetric);
  HHVM_RCC_I(Imagick, METRIC_PEAKABSOLUTEERROR, PeakAbsoluteErrorMetric);
  HHVM_RCC_I(Imagick, METRIC_PEAKSIGNALTONOISERATIO,
                           PeakSignalToNoiseRatioMetric);
  HHVM_RCC_I(Imagick, METRIC_ROOTMEANSQUAREDERROR, RootMeanSquaredErrorMetric);

  // PIXEL constants
  HHVM_RCC_I(Imagick, PIXEL_CHAR, CharPixel);
  HHVM_RCC_I(Imagick, PIXEL_DOUBLE, DoublePixel);
  HHVM_RCC_I(Imagick, PIXEL_FLOAT, FloatPixel);
  HHVM_RCC_I(Imagick, PIXEL_INTEGER, IntegerPixel);
  HHVM_RCC_I(Imagick, PIXEL_LONG, LongPixel);
  HHVM_RCC_I(Imagick, PIXEL_QUANTUM, QuantumPixel);
  HHVM_RCC_I(Imagick, PIXEL_SHORT, ShortPixel);

  // EVALUATE constants
  HHVM_RCC_I(Imagick, EVALUATE_UNDEFINED, UndefinedEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_ADD, AddEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_AND, AndEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_DIVIDE, DivideEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_LEFTSHIFT, LeftShiftEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MAX, MaxEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MIN, MinEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MULTIPLY, MultiplyEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_OR, OrEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_RIGHTSHIFT, RightShiftEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_SET, SetEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_SUBTRACT, SubtractEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_XOR, XorEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_POW, PowEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_LOG, LogEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_THRESHOLD, ThresholdEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_THRESHOLDBLACK, ThresholdBlackEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_THRESHOLDWHITE, ThresholdWhiteEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_GAUSSIANNOISE, GaussianNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_IMPULSENOISE, ImpulseNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_LAPLACIANNOISE, LaplacianNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MULTIPLICATIVENOISE,
                           MultiplicativeNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_POISSONNOISE, PoissonNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_UNIFORMNOISE, UniformNoiseEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_COSINE, CosineEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_SINE, SineEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_ADDMODULUS, AddModulusEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MEAN, MeanEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_ABS, AbsEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_EXPONENTIAL, ExponentialEvaluateOperator);
  HHVM_RCC_I(Imagick, EVALUATE_MEDIAN, MedianEvaluateOperator);
#if MagickLibVersion >= 0x676
  HHVM_RCC_I(Imagick, EVALUATE_SUM, SumEvaluateOperator);
#endif

  // COLORSPACE constants
  HHVM_RCC_I(Imagick, COLORSPACE_UNDEFINED, UndefinedColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_RGB, RGBColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_GRAY, GRAYColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_TRANSPARENT, TransparentColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_OHTA, OHTAColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_LAB, LABColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_XYZ, XYZColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YCBCR, YCbCrColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YCC, YCCColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YIQ, YIQColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YPBPR, YPbPrColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YUV, YUVColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_CMYK, CMYKColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_SRGB, sRGBColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HSB, HSBColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HSL, HSLColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HWB, HWBColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_REC601LUMA, Rec601LumaColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_REC709LUMA, Rec709LumaColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_LOG, LogColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_CMY, CMYColorspace);
#if MagickLibVersion >= 0x679
  HHVM_RCC_I(Imagick, COLORSPACE_LUV, LuvColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HCL, HCLColorspace);
#endif
#if MagickLibVersion >= 0x680
  HHVM_RCC_I(Imagick, COLORSPACE_LCH, LCHColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_LMS, LMSColorspace);
#endif
#if MagickLibVersion >= 0x686
  HHVM_RCC_I(Imagick, COLORSPACE_LCHAB, LCHabColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_LCHUV, LCHuvColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_SCRGB, scRGBColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HSI, HSIColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HSV, HSVColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_HCLP, HCLpColorspace);
  HHVM_RCC_I(Imagick, COLORSPACE_YDBDR, YDbDrColorspace);
#endif

  // VIRTUALPIXELMETHOD constants
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_UNDEFINED,
                           UndefinedVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_BACKGROUND,
                           BackgroundVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_CONSTANT, ConstantVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_EDGE, EdgeVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_MIRROR, MirrorVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_TILE, TileVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_TRANSPARENT,
                           TransparentVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_MASK, MaskVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_BLACK, BlackVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_GRAY, GrayVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_WHITE, WhiteVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_HORIZONTALTILE,
                           HorizontalTileVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_VERTICALTILE,
                           VerticalTileVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_HORIZONTALTILEEDGE,
                           HorizontalTileEdgeVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_VERTICALTILEEDGE,
                           VerticalTileEdgeVirtualPixelMethod);
  HHVM_RCC_I(Imagick, VIRTUALPIXELMETHOD_CHECKERTILE,
                           CheckerTileVirtualPixelMethod);

  // PREVIEW constants
  HHVM_RCC_I(Imagick, PREVIEW_UNDEFINED, UndefinedPreview);
  HHVM_RCC_I(Imagick, PREVIEW_ROTATE, RotatePreview);
  HHVM_RCC_I(Imagick, PREVIEW_SHEAR, ShearPreview);
  HHVM_RCC_I(Imagick, PREVIEW_ROLL, RollPreview);
  HHVM_RCC_I(Imagick, PREVIEW_HUE, HuePreview);
  HHVM_RCC_I(Imagick, PREVIEW_SATURATION, SaturationPreview);
  HHVM_RCC_I(Imagick, PREVIEW_BRIGHTNESS, BrightnessPreview);
  HHVM_RCC_I(Imagick, PREVIEW_GAMMA, GammaPreview);
  HHVM_RCC_I(Imagick, PREVIEW_SPIFF, SpiffPreview);
  HHVM_RCC_I(Imagick, PREVIEW_DULL, DullPreview);
  HHVM_RCC_I(Imagick, PREVIEW_GRAYSCALE, GrayscalePreview);
  HHVM_RCC_I(Imagick, PREVIEW_QUANTIZE, QuantizePreview);
  HHVM_RCC_I(Imagick, PREVIEW_DESPECKLE, DespecklePreview);
  HHVM_RCC_I(Imagick, PREVIEW_REDUCENOISE, ReduceNoisePreview);
  HHVM_RCC_I(Imagick, PREVIEW_ADDNOISE, AddNoisePreview);
  HHVM_RCC_I(Imagick, PREVIEW_SHARPEN, SharpenPreview);
  HHVM_RCC_I(Imagick, PREVIEW_BLUR, BlurPreview);
  HHVM_RCC_I(Imagick, PREVIEW_THRESHOLD, ThresholdPreview);
  HHVM_RCC_I(Imagick, PREVIEW_EDGEDETECT, EdgeDetectPreview);
  HHVM_RCC_I(Imagick, PREVIEW_SPREAD, SpreadPreview);
  HHVM_RCC_I(Imagick, PREVIEW_SOLARIZE, SolarizePreview);
  HHVM_RCC_I(Imagick, PREVIEW_SHADE, ShadePreview);
  HHVM_RCC_I(Imagick, PREVIEW_RAISE, RaisePreview);
  HHVM_RCC_I(Imagick, PREVIEW_SEGMENT, SegmentPreview);
  HHVM_RCC_I(Imagick, PREVIEW_SWIRL, SwirlPreview);
  HHVM_RCC_I(Imagick, PREVIEW_IMPLODE, ImplodePreview);
  HHVM_RCC_I(Imagick, PREVIEW_WAVE, WavePreview);
  HHVM_RCC_I(Imagick, PREVIEW_OILPAINT, OilPaintPreview);
  HHVM_RCC_I(Imagick, PREVIEW_CHARCOALDRAWING, CharcoalDrawingPreview);
  HHVM_RCC_I(Imagick, PREVIEW_JPEG, JPEGPreview);

  // RENDERINGINTENT constants
  HHVM_RCC_I(Imagick, RENDERINGINTENT_UNDEFINED, UndefinedIntent);
  HHVM_RCC_I(Imagick, RENDERINGINTENT_SATURATION, SaturationIntent);
  HHVM_RCC_I(Imagick, RENDERINGINTENT_PERCEPTUAL, PerceptualIntent);
  HHVM_RCC_I(Imagick, RENDERINGINTENT_ABSOLUTE, AbsoluteIntent);
  HHVM_RCC_I(Imagick, RENDERINGINTENT_RELATIVE, RelativeIntent);

  // INTERLACE constants
  HHVM_RCC_I(Imagick, INTERLACE_UNDEFINED, UndefinedInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_NO, NoInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_LINE, LineInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_PLANE, PlaneInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_PARTITION, PartitionInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_GIF, GIFInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_JPEG, JPEGInterlace);
  HHVM_RCC_I(Imagick, INTERLACE_PNG, PNGInterlace);

  // FILLRULE constants
  HHVM_RCC_I(Imagick, FILLRULE_UNDEFINED, UndefinedRule);
  HHVM_RCC_I(Imagick, FILLRULE_EVENODD, EvenOddRule);
  HHVM_RCC_I(Imagick, FILLRULE_NONZERO, NonZeroRule);

  // PATHUNITS constants
  HHVM_RCC_I(Imagick, PATHUNITS_UNDEFINED, UndefinedPathUnits);
  HHVM_RCC_I(Imagick, PATHUNITS_USERSPACE, UserSpace);
  HHVM_RCC_I(Imagick, PATHUNITS_USERSPACEONUSE, UserSpaceOnUse);
  HHVM_RCC_I(Imagick, PATHUNITS_OBJECTBOUNDINGBOX, ObjectBoundingBox);

  // LINECAP constants
  HHVM_RCC_I(Imagick, LINECAP_UNDEFINED, UndefinedCap);
  HHVM_RCC_I(Imagick, LINECAP_BUTT, ButtCap);
  HHVM_RCC_I(Imagick, LINECAP_ROUND, RoundCap);
  HHVM_RCC_I(Imagick, LINECAP_SQUARE, SquareCap);

  // LINEJOIN constants
  HHVM_RCC_I(Imagick, LINEJOIN_UNDEFINED, UndefinedJoin);
  HHVM_RCC_I(Imagick, LINEJOIN_MITER, MiterJoin);
  HHVM_RCC_I(Imagick, LINEJOIN_ROUND, RoundJoin);
  HHVM_RCC_I(Imagick, LINEJOIN_BEVEL, BevelJoin);

  // RESOURCETYPE constants
  HHVM_RCC_I(Imagick, RESOURCETYPE_UNDEFINED, UndefinedResource);
  HHVM_RCC_I(Imagick, RESOURCETYPE_AREA, AreaResource);
  HHVM_RCC_I(Imagick, RESOURCETYPE_DISK, DiskResource);
  HHVM_RCC_I(Imagick, RESOURCETYPE_FILE, FileResource);
  HHVM_RCC_I(Imagick, RESOURCETYPE_MAP, MapResource);
  HHVM_RCC_I(Imagick, RESOURCETYPE_MEMORY, MemoryResource);
#if MagickLibVersion > 0x678
  HHVM_RCC_I(Imagick, RESOURCETYPE_THREAD, ThreadResource);
#endif

  // LAYERMETHOD constants
  HHVM_RCC_I(Imagick, LAYERMETHOD_UNDEFINED, UndefinedLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_COALESCE, CoalesceLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_COMPAREANY, CompareAnyLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_COMPARECLEAR, CompareClearLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_COMPAREOVERLAY, CompareOverlayLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_DISPOSE, DisposeLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_OPTIMIZE, OptimizeLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_OPTIMIZEPLUS, OptimizePlusLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_OPTIMIZETRANS, OptimizeTransLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_COMPOSITE, CompositeLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_OPTIMIZEIMAGE, OptimizeImageLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_REMOVEDUPS, RemoveDupsLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_REMOVEZERO, RemoveZeroLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_MERGE, MergeLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_FLATTEN, FlattenLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_MOSAIC, MosaicLayer);
  HHVM_RCC_I(Imagick, LAYERMETHOD_TRIMBOUNDS, TrimBoundsLayer);

  // ORIENTATION constants
  HHVM_RCC_I(Imagick, ORIENTATION_UNDEFINED, UndefinedOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_TOPLEFT, TopLeftOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_TOPRIGHT, TopRightOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_BOTTOMRIGHT, BottomRightOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_BOTTOMLEFT, BottomLeftOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_LEFTTOP, LeftTopOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_RIGHTTOP, RightTopOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_RIGHTBOTTOM, RightBottomOrientation);
  HHVM_RCC_I(Imagick, ORIENTATION_LEFTBOTTOM, LeftBottomOrientation);

  // DISTORTION constants
  HHVM_RCC_I(Imagick, DISTORTION_UNDEFINED, UndefinedDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_AFFINE, AffineDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_AFFINEPROJECTION, AffineProjectionDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_ARC, ArcDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_BILINEAR, BilinearDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_PERSPECTIVE, PerspectiveDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_PERSPECTIVEPROJECTION,
                           PerspectiveProjectionDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_SCALEROTATETRANSLATE,
                           ScaleRotateTranslateDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_POLYNOMIAL, PolynomialDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_POLAR, PolarDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_DEPOLAR, DePolarDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_BARREL, BarrelDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_BARRELINVERSE, BarrelInverseDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_SHEPARDS, ShepardsDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_SENTINEL, SentinelDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_BILINEARFORWARD, BilinearForwardDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_BILINEARREVERSE, BilinearReverseDistortion);
#if MagickLibVersion >= 0x670
  HHVM_RCC_I(Imagick, DISTORTION_RESIZE, ResizeDistortion);
#endif
#if MagickLibVersion >= 0x671
  HHVM_RCC_I(Imagick, DISTORTION_CYLINDER2PLANE, Cylinder2PlaneDistortion);
  HHVM_RCC_I(Imagick, DISTORTION_PLANE2CYLINDER, Plane2CylinderDistortion);
#endif

  // ALPHACHANNEL constants
  HHVM_RCC_I(Imagick, ALPHACHANNEL_ACTIVATE, ActivateAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_DEACTIVATE, DeactivateAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_RESET, ResetAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_SET, SetAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_UNDEFINED, UndefinedAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_COPY, CopyAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_EXTRACT, ExtractAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_OPAQUE, OpaqueAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_SHAPE, ShapeAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_TRANSPARENT,
                           TransparentAlphaChannel);
#if MagickLibVersion >= 0x680
  HHVM_RCC_I(Imagick, ALPHACHANNEL_FLATTEN, FlattenAlphaChannel);
  HHVM_RCC_I(Imagick, ALPHACHANNEL_REMOVE, RemoveAlphaChannel);
#endif

  // SPARSECOLORMETHOD constants
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_UNDEFINED, UndefinedColorInterpolate);
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_BARYCENTRIC,
                           BarycentricColorInterpolate);
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_BILINEAR, BilinearColorInterpolate);
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_POLYNOMIAL, PolynomialColorInterpolate);
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_SPEPARDS, ShepardsColorInterpolate);
  HHVM_RCC_I(Imagick, SPARSECOLORMETHOD_VORONOI, VoronoiColorInterpolate);

  // FUNCTION constants
  HHVM_RCC_I(Imagick, FUNCTION_UNDEFINED, UndefinedFunction);
  HHVM_RCC_I(Imagick, FUNCTION_POLYNOMIAL, PolynomialFunction);
  HHVM_RCC_I(Imagick, FUNCTION_SINUSOID, SinusoidFunction);
  HHVM_RCC_I(Imagick, FUNCTION_ARCSIN, ArcsinFunction);
  HHVM_RCC_I(Imagick, FUNCTION_ARCTAN, ArctanFunction);

  // INTERPOLATE constants
  HHVM_RCC_I(Imagick, INTERPOLATE_UNDEFINED, UndefinedInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_AVERAGE, AverageInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_BICUBIC, BicubicInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_BILINEAR, BilinearInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_FILTER, FilterInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_INTEGER, IntegerInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_MESH, MeshInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_NEARESTNEIGHBOR,
                           NearestNeighborInterpolatePixel);
  HHVM_RCC_I(Imagick, INTERPOLATE_SPLINE, SplineInterpolatePixel);

  // DITHERMETHOD constants
  HHVM_RCC_I(Imagick, DITHERMETHOD_UNDEFINED, UndefinedDitherMethod);
  HHVM_RCC_I(Imagick, DITHERMETHOD_NO, NoDitherMethod);
  HHVM_RCC_I(Imagick, DITHERMETHOD_RIEMERSMA, RiemersmaDitherMethod);
  HHVM_RCC_I(Imagick, DITHERMETHOD_FLOYDSTEINBERG, FloydSteinbergDitherMethod);

  // Global constants
  HHVM_RC_I(MW_AbsoluteIntent, AbsoluteIntent);
  HHVM_RC_I(MW_AddCompositeOp, AddCompositeOp);
  HHVM_RC_I(MW_AddEvaluateOperator, AddEvaluateOperator);
  HHVM_RC_I(MW_AddNoisePreview, AddNoisePreview);
  HHVM_RC_I(MW_AllChannels, AllChannels);
  HHVM_RC_I(MW_AlphaChannel, AlphaChannel);
  HHVM_RC_I(MW_AndEvaluateOperator, AndEvaluateOperator);
  HHVM_RC_I(MW_AnyStretch, AnyStretch);
  HHVM_RC_I(MW_AnyStyle, AnyStyle);
  HHVM_RC_I(MW_AreaResource, AreaResource);
  HHVM_RC_I(MW_AtopCompositeOp, AtopCompositeOp);
  HHVM_RC_I(MW_BZipCompression, BZipCompression);
  HHVM_RC_I(MW_BackgroundDispose, BackgroundDispose);
  HHVM_RC_I(MW_BesselFilter, BesselFilter);
  HHVM_RC_I(MW_BevelJoin, BevelJoin);
  HHVM_RC_I(MW_BilevelType, BilevelType);
  HHVM_RC_I(MW_BlackChannel, BlackChannel);
  HHVM_RC_I(MW_BlackmanFilter, BlackmanFilter);
  HHVM_RC_I(MW_BlendCompositeOp, BlendCompositeOp);
  HHVM_RC_I(MW_BlobError, BlobError);
  HHVM_RC_I(MW_BlobFatalError, BlobFatalError);
  HHVM_RC_I(MW_BlobWarning, BlobWarning);
  HHVM_RC_I(MW_BlueChannel, BlueChannel);
  HHVM_RC_I(MW_BlurPreview, BlurPreview);
  HHVM_RC_I(MW_BoxFilter, BoxFilter);
  HHVM_RC_I(MW_BrightnessPreview, BrightnessPreview);
  HHVM_RC_I(MW_BumpmapCompositeOp, BumpmapCompositeOp);
  HHVM_RC_I(MW_ButtCap, ButtCap);
  HHVM_RC_I(MW_CMYKColorspace, CMYKColorspace);
  HHVM_RC_I(MW_CacheError, CacheError);
  HHVM_RC_I(MW_CacheFatalError, CacheFatalError);
  HHVM_RC_I(MW_CacheWarning, CacheWarning);
  HHVM_RC_I(MW_CatromFilter, CatromFilter);
  HHVM_RC_I(MW_CenterAlign, CenterAlign);
  HHVM_RC_I(MW_CenterGravity, CenterGravity);
  HHVM_RC_I(MW_CharPixel, CharPixel);
  HHVM_RC_I(MW_CharcoalDrawingPreview, CharcoalDrawingPreview);
  HHVM_RC_I(MW_ClearCompositeOp, ClearCompositeOp);
  HHVM_RC_I(MW_CoderError, CoderError);
  HHVM_RC_I(MW_CoderFatalError, CoderFatalError);
  HHVM_RC_I(MW_CoderWarning, CoderWarning);
  HHVM_RC_I(MW_ColorBurnCompositeOp, ColorBurnCompositeOp);
  HHVM_RC_I(MW_ColorDodgeCompositeOp, ColorDodgeCompositeOp);
  HHVM_RC_I(MW_ColorSeparationMatteType, ColorSeparationMatteType);
  HHVM_RC_I(MW_ColorSeparationType, ColorSeparationType);
  HHVM_RC_I(MW_ColorizeCompositeOp, ColorizeCompositeOp);
  HHVM_RC_I(MW_ConcatenateMode, ConcatenateMode);
  HHVM_RC_I(MW_CondensedStretch, CondensedStretch);
  HHVM_RC_I(MW_ConfigureError, ConfigureError);
  HHVM_RC_I(MW_ConfigureFatalError, ConfigureFatalError);
  HHVM_RC_I(MW_ConfigureWarning, ConfigureWarning);
  HHVM_RC_I(MW_ConstantVirtualPixelMethod, ConstantVirtualPixelMethod);
  HHVM_RC_I(MW_CopyBlackCompositeOp, CopyBlackCompositeOp);
  HHVM_RC_I(MW_CopyBlueCompositeOp, CopyBlueCompositeOp);
  HHVM_RC_I(MW_CopyCompositeOp, CopyCompositeOp);
  HHVM_RC_I(MW_CopyCyanCompositeOp, CopyCyanCompositeOp);
  HHVM_RC_I(MW_CopyGreenCompositeOp, CopyGreenCompositeOp);
  HHVM_RC_I(MW_CopyMagentaCompositeOp, CopyMagentaCompositeOp);
  HHVM_RC_I(MW_CopyOpacityCompositeOp, CopyOpacityCompositeOp);
  HHVM_RC_I(MW_CopyRedCompositeOp, CopyRedCompositeOp);
  HHVM_RC_I(MW_CopyYellowCompositeOp, CopyYellowCompositeOp);
  HHVM_RC_I(MW_CorruptImageError, CorruptImageError);
  HHVM_RC_I(MW_CorruptImageFatalError, CorruptImageFatalError);
  HHVM_RC_I(MW_CorruptImageWarning, CorruptImageWarning);
  HHVM_RC_I(MW_CubicFilter, CubicFilter);
  HHVM_RC_I(MW_CyanChannel, CyanChannel);
  HHVM_RC_I(MW_DarkenCompositeOp, DarkenCompositeOp);
  HHVM_RC_I(MW_DelegateError, DelegateError);
  HHVM_RC_I(MW_DelegateFatalError, DelegateFatalError);
  HHVM_RC_I(MW_DelegateWarning, DelegateWarning);
  HHVM_RC_I(MW_DespecklePreview, DespecklePreview);
  HHVM_RC_I(MW_DifferenceCompositeOp, DifferenceCompositeOp);
  HHVM_RC_I(MW_DiskResource, DiskResource);
  HHVM_RC_I(MW_DisplaceCompositeOp, DisplaceCompositeOp);
  HHVM_RC_I(MW_DissolveCompositeOp, DissolveCompositeOp);
  HHVM_RC_I(MW_DivideEvaluateOperator, DivideEvaluateOperator);
  HHVM_RC_I(MW_DoublePixel, DoublePixel);
  HHVM_RC_I(MW_DrawError, DrawError);
  HHVM_RC_I(MW_DrawFatalError, DrawFatalError);
  HHVM_RC_I(MW_DrawWarning, DrawWarning);
  HHVM_RC_I(MW_DstAtopCompositeOp, DstAtopCompositeOp);
  HHVM_RC_I(MW_DstCompositeOp, DstCompositeOp);
  HHVM_RC_I(MW_DstInCompositeOp, DstInCompositeOp);
  HHVM_RC_I(MW_DstOutCompositeOp, DstOutCompositeOp);
  HHVM_RC_I(MW_DstOverCompositeOp, DstOverCompositeOp);
  HHVM_RC_I(MW_DullPreview, DullPreview);
  HHVM_RC_I(MW_EastGravity, EastGravity);
  HHVM_RC_I(MW_EdgeDetectPreview, EdgeDetectPreview);
  HHVM_RC_I(MW_EdgeVirtualPixelMethod, EdgeVirtualPixelMethod);
  HHVM_RC_I(MW_ErrorException, ErrorException);
  HHVM_RC_I(MW_EvenOddRule, EvenOddRule);
  HHVM_RC_I(MW_ExclusionCompositeOp, ExclusionCompositeOp);
  HHVM_RC_I(MW_ExpandedStretch, ExpandedStretch);
  HHVM_RC_I(MW_ExtraCondensedStretch, ExtraCondensedStretch);
  HHVM_RC_I(MW_ExtraExpandedStretch, ExtraExpandedStretch);
  HHVM_RC_I(MW_FatalErrorException, ::FatalErrorException);
  HHVM_RC_I(MW_FaxCompression, FaxCompression);
  HHVM_RC_I(MW_FileOpenError, FileOpenError);
  HHVM_RC_I(MW_FileOpenFatalError, FileOpenFatalError);
  HHVM_RC_I(MW_FileOpenWarning, FileOpenWarning);
  HHVM_RC_I(MW_FileResource, FileResource);
  HHVM_RC_I(MW_FillToBorderMethod, FillToBorderMethod);
  HHVM_RC_I(MW_FloatPixel, FloatPixel);
  HHVM_RC_I(MW_FloodfillMethod, FloodfillMethod);
  HHVM_RC_I(MW_ForgetGravity, ForgetGravity);
  HHVM_RC_I(MW_FrameMode, FrameMode);
  HHVM_RC_I(MW_GRAYColorspace, GRAYColorspace);
  HHVM_RC_I(MW_GammaPreview, GammaPreview);
  HHVM_RC_I(MW_GaussianFilter, GaussianFilter);
  HHVM_RC_I(MW_GaussianNoise, GaussianNoise);
  HHVM_RC_I(MW_GrayscaleMatteType, GrayscaleMatteType);
  HHVM_RC_I(MW_GrayscalePreview, GrayscalePreview);
  HHVM_RC_I(MW_GrayscaleType, GrayscaleType);
  HHVM_RC_I(MW_GreenChannel, GreenChannel);
  HHVM_RC_I(MW_Group4Compression, Group4Compression);
  HHVM_RC_I(MW_HSBColorspace, HSBColorspace);
  HHVM_RC_I(MW_HSLColorspace, HSLColorspace);
  HHVM_RC_I(MW_HWBColorspace, HWBColorspace);
  HHVM_RC_I(MW_HammingFilter, HammingFilter);
  HHVM_RC_I(MW_HanningFilter, HanningFilter);
  HHVM_RC_I(MW_HardLightCompositeOp, HardLightCompositeOp);
  HHVM_RC_I(MW_HermiteFilter, HermiteFilter);
  HHVM_RC_I(MW_HueCompositeOp, HueCompositeOp);
  HHVM_RC_I(MW_HuePreview, HuePreview);
  HHVM_RC_I(MW_ImageError, ImageError);
  HHVM_RC_I(MW_ImageFatalError, ImageFatalError);
  HHVM_RC_I(MW_ImageWarning, ImageWarning);
  HHVM_RC_I(MW_ImplodePreview, ImplodePreview);
  HHVM_RC_I(MW_ImpulseNoise, ImpulseNoise);
  HHVM_RC_I(MW_InCompositeOp, InCompositeOp);
  HHVM_RC_I(MW_IndexChannel, IndexChannel);
  HHVM_RC_I(MW_IntegerPixel, IntegerPixel);
  HHVM_RC_I(MW_ItalicStyle, ItalicStyle);
  HHVM_RC_I(MW_JPEGCompression, JPEGCompression);
  HHVM_RC_I(MW_JPEGPreview, JPEGPreview);
  HHVM_RC_I(MW_LABColorspace, LABColorspace);
  HHVM_RC_I(MW_LZWCompression, LZWCompression);
  HHVM_RC_I(MW_LanczosFilter, LanczosFilter);
  HHVM_RC_I(MW_LaplacianNoise, LaplacianNoise);
  HHVM_RC_I(MW_LeftAlign, LeftAlign);
  HHVM_RC_I(MW_LeftShiftEvaluateOperator, LeftShiftEvaluateOperator);
  HHVM_RC_I(MW_LightenCompositeOp, LightenCompositeOp);
  HHVM_RC_I(MW_LineInterlace, LineInterlace);
  HHVM_RC_I(MW_LineThroughDecoration, LineThroughDecoration);
  HHVM_RC_I(MW_LongPixel, LongPixel);
  HHVM_RC_I(MW_LosslessJPEGCompression, LosslessJPEGCompression);
  HHVM_RC_I(MW_LuminizeCompositeOp, LuminizeCompositeOp);
  HHVM_RC_I(MW_MagentaChannel, MagentaChannel);
  HHVM_RC_I(MW_MapResource, MapResource);
  HHVM_RC_I(MW_MaxEvaluateOperator, MaxEvaluateOperator);
  HHVM_RC_I(MW_MeanAbsoluteErrorMetric, MeanAbsoluteErrorMetric);
  HHVM_RC_I(MW_MeanSquaredErrorMetric, MeanSquaredErrorMetric);
  HHVM_RC_I(MW_MemoryResource, MemoryResource);
  HHVM_RC_I(MW_MinEvaluateOperator, MinEvaluateOperator);
  HHVM_RC_I(MW_MinusCompositeOp, MinusCompositeOp);
  HHVM_RC_I(MW_MirrorVirtualPixelMethod, MirrorVirtualPixelMethod);
  HHVM_RC_I(MW_MissingDelegateError, MissingDelegateError);
  HHVM_RC_I(MW_MissingDelegateFatalError, MissingDelegateFatalError);
  HHVM_RC_I(MW_MissingDelegateWarning, MissingDelegateWarning);
  HHVM_RC_I(MW_MitchellFilter, MitchellFilter);
  HHVM_RC_I(MW_MiterJoin, MiterJoin);
  HHVM_RC_I(MW_ModulateCompositeOp, ModulateCompositeOp);
  HHVM_RC_I(MW_ModuleError, ModuleError);
  HHVM_RC_I(MW_ModuleFatalError, ModuleFatalError);
  HHVM_RC_I(MW_ModuleWarning, ModuleWarning);
  HHVM_RC_I(MW_MonitorError, MonitorError);
  HHVM_RC_I(MW_MonitorFatalError, MonitorFatalError);
  HHVM_RC_I(MW_MonitorWarning, MonitorWarning);
  HHVM_RC_I(MW_MultiplicativeGaussianNoise, MultiplicativeGaussianNoise);
  HHVM_RC_I(MW_MultiplyCompositeOp, MultiplyCompositeOp);
  HHVM_RC_I(MW_MultiplyEvaluateOperator, MultiplyEvaluateOperator);
  HHVM_RC_I(MW_NoCompositeOp, NoCompositeOp);
  HHVM_RC_I(MW_NoCompression, NoCompression);
  HHVM_RC_I(MW_NoDecoration, NoDecoration);
  HHVM_RC_I(MW_NoInterlace, NoInterlace);
  HHVM_RC_I(MW_NonZeroRule, NonZeroRule);
  HHVM_RC_I(MW_NoneDispose, NoneDispose);
  HHVM_RC_I(MW_NormalStretch, NormalStretch);
  HHVM_RC_I(MW_NormalStyle, NormalStyle);
  HHVM_RC_I(MW_NorthEastGravity, NorthEastGravity);
  HHVM_RC_I(MW_NorthGravity, NorthGravity);
  HHVM_RC_I(MW_NorthWestGravity, NorthWestGravity);
  HHVM_RC_I(MW_OHTAColorspace, OHTAColorspace);
  HHVM_RC_I(MW_ObjectBoundingBox, ObjectBoundingBox);
  HHVM_RC_I(MW_ObliqueStyle, ObliqueStyle);
  HHVM_RC_I(MW_OilPaintPreview, OilPaintPreview);
  HHVM_RC_I(MW_OpacityChannel, OpacityChannel);
  HHVM_RC_I(MW_OptimizeType, OptimizeType);
  HHVM_RC_I(MW_OptionError, OptionError);
  HHVM_RC_I(MW_OptionFatalError, OptionFatalError);
  HHVM_RC_I(MW_OptionWarning, OptionWarning);
  HHVM_RC_I(MW_OrEvaluateOperator, OrEvaluateOperator);
  HHVM_RC_I(MW_OutCompositeOp, OutCompositeOp);
  HHVM_RC_I(MW_OverCompositeOp, OverCompositeOp);
  HHVM_RC_I(MW_OverlayCompositeOp, OverlayCompositeOp);
  HHVM_RC_I(MW_OverlineDecoration, OverlineDecoration);
  HHVM_RC_I(MW_PaletteMatteType, PaletteMatteType);
  HHVM_RC_I(MW_PaletteType, PaletteType);
  HHVM_RC_I(MW_PartitionInterlace, PartitionInterlace);
  HHVM_RC_I(MW_PeakAbsoluteErrorMetric, PeakAbsoluteErrorMetric);
  HHVM_RC_I(MW_PeakSignalToNoiseRatioMetric, PeakSignalToNoiseRatioMetric);
  HHVM_RC_I(MW_PerceptualIntent, PerceptualIntent);
  HHVM_RC_I(MW_PixelsPerCentimeterResolution, PixelsPerCentimeterResolution);
  HHVM_RC_I(MW_PixelsPerInchResolution, PixelsPerInchResolution);
  HHVM_RC_I(MW_PlaneInterlace, PlaneInterlace);
  HHVM_RC_I(MW_PlusCompositeOp, PlusCompositeOp);
  HHVM_RC_I(MW_PointFilter, PointFilter);
  HHVM_RC_I(MW_PointMethod, PointMethod);
  HHVM_RC_I(MW_PoissonNoise, PoissonNoise);
  HHVM_RC_I(MW_PreviousDispose, PreviousDispose);
  HHVM_RC_I(MW_QuadraticFilter, QuadraticFilter);
  HHVM_RC_I(MW_QuantizePreview, QuantizePreview);
  HHVM_RC_I(MW_RGBColorspace, RGBColorspace);
  HHVM_RC_I(MW_RLECompression, RLECompression);
  HHVM_RC_I(MW_RaisePreview, RaisePreview);
  HHVM_RC_I(MW_RedChannel, RedChannel);
  HHVM_RC_I(MW_ReduceNoisePreview, ReduceNoisePreview);
  HHVM_RC_I(MW_RegistryError, RegistryError);
  HHVM_RC_I(MW_RegistryFatalError, RegistryFatalError);
  HHVM_RC_I(MW_RegistryWarning, RegistryWarning);
  HHVM_RC_I(MW_RelativeIntent, RelativeIntent);
  HHVM_RC_I(MW_ReplaceCompositeOp, ReplaceCompositeOp);
  HHVM_RC_I(MW_ReplaceMethod, ReplaceMethod);
  HHVM_RC_I(MW_ResetMethod, ResetMethod);
  HHVM_RC_I(MW_ResourceLimitError, ResourceLimitError);
  HHVM_RC_I(MW_ResourceLimitFatalError, ResourceLimitFatalError);
  HHVM_RC_I(MW_ResourceLimitWarning, ResourceLimitWarning);
  HHVM_RC_I(MW_RightAlign, RightAlign);
  HHVM_RC_I(MW_RightShiftEvaluateOperator, RightShiftEvaluateOperator);
  HHVM_RC_I(MW_RollPreview, RollPreview);
  HHVM_RC_I(MW_RootMeanSquaredErrorMetric, RootMeanSquaredErrorMetric);
  HHVM_RC_I(MW_RotatePreview, RotatePreview);
  HHVM_RC_I(MW_RoundCap, RoundCap);
  HHVM_RC_I(MW_RoundJoin, RoundJoin);
  HHVM_RC_I(MW_SaturateCompositeOp, SaturateCompositeOp);
  HHVM_RC_I(MW_SaturationIntent, SaturationIntent);
  HHVM_RC_I(MW_SaturationPreview, SaturationPreview);
  HHVM_RC_I(MW_ScreenCompositeOp, ScreenCompositeOp);
  HHVM_RC_I(MW_SegmentPreview, SegmentPreview);
  HHVM_RC_I(MW_SemiCondensedStretch, SemiCondensedStretch);
  HHVM_RC_I(MW_SemiExpandedStretch, SemiExpandedStretch);
  HHVM_RC_I(MW_SetEvaluateOperator, SetEvaluateOperator);
  HHVM_RC_I(MW_ShadePreview, ShadePreview);
  HHVM_RC_I(MW_SharpenPreview, SharpenPreview);
  HHVM_RC_I(MW_ShearPreview, ShearPreview);
  HHVM_RC_I(MW_ShortPixel, ShortPixel);
  HHVM_RC_I(MW_SincFilter, SincFilter);
  HHVM_RC_I(MW_SoftLightCompositeOp, SoftLightCompositeOp);
  HHVM_RC_I(MW_SolarizePreview, SolarizePreview);
  HHVM_RC_I(MW_SouthEastGravity, SouthEastGravity);
  HHVM_RC_I(MW_SouthGravity, SouthGravity);
  HHVM_RC_I(MW_SouthWestGravity, SouthWestGravity);
  HHVM_RC_I(MW_SpiffPreview, SpiffPreview);
  HHVM_RC_I(MW_SpreadPreview, SpreadPreview);
  HHVM_RC_I(MW_SquareCap, SquareCap);
  HHVM_RC_I(MW_SrcAtopCompositeOp, SrcAtopCompositeOp);
  HHVM_RC_I(MW_SrcCompositeOp, SrcCompositeOp);
  HHVM_RC_I(MW_SrcInCompositeOp, SrcInCompositeOp);
  HHVM_RC_I(MW_SrcOutCompositeOp, SrcOutCompositeOp);
  HHVM_RC_I(MW_SrcOverCompositeOp, SrcOverCompositeOp);
  HHVM_RC_I(MW_StaticGravity, StaticGravity);
  HHVM_RC_I(MW_StreamError, StreamError);
  HHVM_RC_I(MW_StreamFatalError, StreamFatalError);
  HHVM_RC_I(MW_StreamWarning, StreamWarning);
  HHVM_RC_I(MW_SubtractCompositeOp, SubtractCompositeOp);
  HHVM_RC_I(MW_SubtractEvaluateOperator, SubtractEvaluateOperator);
  HHVM_RC_I(MW_SwirlPreview, SwirlPreview);
  HHVM_RC_I(MW_ThresholdCompositeOp, ThresholdCompositeOp);
  HHVM_RC_I(MW_ThresholdPreview, ThresholdPreview);
  HHVM_RC_I(MW_TileVirtualPixelMethod, TileVirtualPixelMethod);
  HHVM_RC_I(MW_TransparentColorspace, TransparentColorspace);
  HHVM_RC_I(MW_TriangleFilter, TriangleFilter);
  HHVM_RC_I(MW_TrueColorMatteType, TrueColorMatteType);
  HHVM_RC_I(MW_TrueColorType, TrueColorType);
  HHVM_RC_I(MW_TypeError, TypeError);
  HHVM_RC_I(MW_TypeFatalError, TypeFatalError);
  HHVM_RC_I(MW_TypeWarning, TypeWarning);
  HHVM_RC_I(MW_UltraCondensedStretch, UltraCondensedStretch);
  HHVM_RC_I(MW_UltraExpandedStretch, UltraExpandedStretch);
  HHVM_RC_I(MW_UndefinedAlign, UndefinedAlign);
  HHVM_RC_I(MW_UndefinedCap, UndefinedCap);
  HHVM_RC_I(MW_UndefinedChannel, UndefinedChannel);
  HHVM_RC_I(MW_UndefinedColorspace, UndefinedColorspace);
  HHVM_RC_I(MW_UndefinedCompositeOp, UndefinedCompositeOp);
  HHVM_RC_I(MW_UndefinedCompression, UndefinedCompression);
  HHVM_RC_I(MW_UndefinedDecoration, UndefinedDecoration);
  HHVM_RC_I(MW_UndefinedDispose, UndefinedDispose);
  HHVM_RC_I(MW_UndefinedEvaluateOperator, UndefinedEvaluateOperator);
  HHVM_RC_I(MW_UndefinedException, UndefinedException);
  HHVM_RC_I(MW_UndefinedFilter, UndefinedFilter);
  HHVM_RC_I(MW_UndefinedGravity, UndefinedGravity);
  HHVM_RC_I(MW_UndefinedIntent, UndefinedIntent);
  HHVM_RC_I(MW_UndefinedInterlace, UndefinedInterlace);
  HHVM_RC_I(MW_UndefinedJoin, UndefinedJoin);
  HHVM_RC_I(MW_UndefinedMethod, UndefinedMethod);
  HHVM_RC_I(MW_UndefinedMetric, UndefinedMetric);
  HHVM_RC_I(MW_UndefinedMode, UndefinedMode);
  HHVM_RC_I(MW_UndefinedNoise, UndefinedNoise);
  HHVM_RC_I(MW_UndefinedPathUnits, UndefinedPathUnits);
  HHVM_RC_I(MW_UndefinedPixel, UndefinedPixel);
  HHVM_RC_I(MW_UndefinedPreview, UndefinedPreview);
  HHVM_RC_I(MW_UndefinedResolution, UndefinedResolution);
  HHVM_RC_I(MW_UndefinedResource, UndefinedResource);
  HHVM_RC_I(MW_UndefinedRule, UndefinedRule);
  HHVM_RC_I(MW_UndefinedStretch, UndefinedStretch);
  HHVM_RC_I(MW_UndefinedStyle, UndefinedStyle);
  HHVM_RC_I(MW_UndefinedType, UndefinedType);
  HHVM_RC_I(MW_UndefinedVirtualPixelMethod, UndefinedVirtualPixelMethod);
  HHVM_RC_I(MW_UnderlineDecoration, UnderlineDecoration);
  HHVM_RC_I(MW_UnframeMode, UnframeMode);
  HHVM_RC_I(MW_UniformNoise, UniformNoise);
  HHVM_RC_I(MW_UnrecognizedDispose, UnrecognizedDispose);
  HHVM_RC_I(MW_UserSpace, UserSpace);
  HHVM_RC_I(MW_UserSpaceOnUse, UserSpaceOnUse);
  HHVM_RC_I(MW_WandError, WandError);
  HHVM_RC_I(MW_WandFatalError, WandFatalError);
  HHVM_RC_I(MW_WandWarning, WandWarning);
  HHVM_RC_I(MW_WarningException, WarningException);
  HHVM_RC_I(MW_WavePreview, WavePreview);
  HHVM_RC_I(MW_WestGravity, WestGravity);
  HHVM_RC_I(MW_XYZColorspace, XYZColorspace);
  HHVM_RC_I(MW_XorCompositeOp, XorCompositeOp);
  HHVM_RC_I(MW_XorEvaluateOperator, XorEvaluateOperator);
  HHVM_RC_I(MW_YCCColorspace, YCCColorspace);
  HHVM_RC_I(MW_YCbCrColorspace, YCbCrColorspace);
  HHVM_RC_I(MW_YIQColorspace, YIQColorspace);
  HHVM_RC_I(MW_YPbPrColorspace, YPbPrColorspace);
  HHVM_RC_I(MW_YUVColorspace, YUVColorspace);
  HHVM_RC_I(MW_YellowChannel, YellowChannel);
  HHVM_RC_I(MW_ZipCompression, ZipCompression);
  HHVM_RC_I(MW_sRGBColorspace, sRGBColorspace);

  size_t range;
  MagickGetQuantumRange(&range);
  HHVM_RC_I(MW_MaxRGB, range);
  HHVM_RC_I(MW_QuantumRange, range);
  HHVM_RC_I(MW_TransparentOpacity, range);

  HHVM_RC_I(MW_OpaqueOpacity, 0);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
