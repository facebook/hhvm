<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class Imagick
  implements Countable, Iterator<Imagick>, Traversable<Imagick> {

  // Constants
  const int COMPOSITE_MODULUSSUBTRACT = 52;
  const int COMPOSITE_DARKENINTENSITY = 66;
  const int COMPOSITE_LIGHTENINTENSITY = 67;
  const int IMGTYPE_COLORSEPARATIONMATTE = 9;
  const int IMGTYPE_PALETTEBILEVELMATTE = 11;
  const int RESOLUTION_PIXELSPERINCH = 1;
  const int RESOLUTION_PIXELSPERCENTIMETER = 2;
  const int COMPRESSION_LOSSLESSJPEG = 10;
  const int NOISE_MULTIPLICATIVEGAUSSIAN = 3;
  const int METRIC_MEANABSOLUTEERROR = 2;
  const int METRIC_PEAKABSOLUTEERROR = 5;
  const int METRIC_PEAKSIGNALTONOISERATIO = 6;
  const int METRIC_ROOTMEANSQUAREDERROR = 7;
  const int EVALUATE_MULTIPLICATIVENOISE = 21;
  const int VIRTUALPIXELMETHOD_UNDEFINED = 0;
  const int VIRTUALPIXELMETHOD_BACKGROUND = 1;
  const int VIRTUALPIXELMETHOD_CONSTANT = 2;
  const int VIRTUALPIXELMETHOD_MIRROR = 5;
  const int VIRTUALPIXELMETHOD_TRANSPARENT = 8;
  const int VIRTUALPIXELMETHOD_BLACK = 10;
  const int VIRTUALPIXELMETHOD_WHITE = 12;
  const int VIRTUALPIXELMETHOD_HORIZONTALTILE = 13;
  const int VIRTUALPIXELMETHOD_VERTICALTILE = 14;
  const int VIRTUALPIXELMETHOD_HORIZONTALTILEEDGE = 15;
  const int VIRTUALPIXELMETHOD_VERTICALTILEEDGE = 16;
  const int VIRTUALPIXELMETHOD_CHECKERTILE = 17;
  const int RENDERINGINTENT_UNDEFINED = 0;
  const int RENDERINGINTENT_SATURATION = 1;
  const int RENDERINGINTENT_PERCEPTUAL = 2;
  const int RENDERINGINTENT_ABSOLUTE = 3;
  const int RENDERINGINTENT_RELATIVE = 4;
  const int PATHUNITS_USERSPACEONUSE = 2;
  const int PATHUNITS_OBJECTBOUNDINGBOX = 3;
  const int LAYERMETHOD_COMPARECLEAR = 3;
  const int LAYERMETHOD_COMPAREOVERLAY = 4;
  const int LAYERMETHOD_OPTIMIZEPLUS = 8;
  const int LAYERMETHOD_OPTIMIZETRANS = 9;
  const int LAYERMETHOD_OPTIMIZEIMAGE = 7;
  const int DISTORTION_AFFINEPROJECTION = 2;
  const int DISTORTION_PERSPECTIVEPROJECTION = 5;
  const int DISTORTION_SCALEROTATETRANSLATE = 3;
  const int DISTORTION_BARRELINVERSE = 15;
  const int DISTORTION_BILINEARFORWARD = 6;
  const int DISTORTION_BILINEARREVERSE = 7;
  const int DISTORTION_CYLINDER2PLANE = 12;
  const int DISTORTION_PLANE2CYLINDER = 13;
  const int ALPHACHANNEL_TRANSPARENT = 10;
  const int SPARSECOLORMETHOD_UNDEFINED = 0;
  const int SPARSECOLORMETHOD_BARYCENTRIC = 1;
  const int SPARSECOLORMETHOD_BILINEAR = 7;
  const int SPARSECOLORMETHOD_POLYNOMIAL = 8;
  const int SPARSECOLORMETHOD_SPEPARDS = 16;
  const int SPARSECOLORMETHOD_VORONOI = 18;
  const int INTERPOLATE_NEARESTNEIGHBOR = 7;
  const int DITHERMETHOD_FLOYDSTEINBERG = 3;
  const int COLOR_BLACK = 0;
  const int COLOR_BLUE = 1;
  const int COLOR_CYAN = 2;
  const int COLOR_GREEN = 3;
  const int COLOR_RED = 4;
  const int COLOR_YELLOW = 5;
  const int COLOR_MAGENTA = 6;
  const int COLOR_OPACITY = 7;
  const int COLOR_ALPHA = 8;
  const int COLOR_FUZZ = 9;
  const int DISPOSE_UNRECOGNIZED = 0;
  const int DISPOSE_UNDEFINED = 0;
  const int DISPOSE_NONE = 1;
  const int DISPOSE_BACKGROUND = 2;
  const int DISPOSE_PREVIOUS = 3;
  const int COMPOSITE_DEFAULT = 40;
  const int COMPOSITE_UNDEFINED = 0;
  const int COMPOSITE_NO = 1;
  const int COMPOSITE_ADD = 2;
  const int COMPOSITE_ATOP = 3;
  const int COMPOSITE_BLEND = 4;
  const int COMPOSITE_BUMPMAP = 5;
  const int COMPOSITE_CLEAR = 7;
  const int COMPOSITE_COLORBURN = 8;
  const int COMPOSITE_COLORDODGE = 9;
  const int COMPOSITE_COLORIZE = 10;
  const int COMPOSITE_COPYBLACK = 11;
  const int COMPOSITE_COPYBLUE = 12;
  const int COMPOSITE_COPY = 13;
  const int COMPOSITE_COPYCYAN = 14;
  const int COMPOSITE_COPYGREEN = 15;
  const int COMPOSITE_COPYMAGENTA = 16;
  const int COMPOSITE_COPYOPACITY = 17;
  const int COMPOSITE_COPYRED = 18;
  const int COMPOSITE_COPYYELLOW = 19;
  const int COMPOSITE_DARKEN = 20;
  const int COMPOSITE_DSTATOP = 21;
  const int COMPOSITE_DST = 22;
  const int COMPOSITE_DSTIN = 23;
  const int COMPOSITE_DSTOUT = 24;
  const int COMPOSITE_DSTOVER = 25;
  const int COMPOSITE_DIFFERENCE = 26;
  const int COMPOSITE_DISPLACE = 27;
  const int COMPOSITE_DISSOLVE = 28;
  const int COMPOSITE_EXCLUSION = 29;
  const int COMPOSITE_HARDLIGHT = 30;
  const int COMPOSITE_HUE = 31;
  const int COMPOSITE_IN = 32;
  const int COMPOSITE_LIGHTEN = 33;
  const int COMPOSITE_LUMINIZE = 35;
  const int COMPOSITE_MINUS = 36;
  const int COMPOSITE_MODULATE = 37;
  const int COMPOSITE_MULTIPLY = 38;
  const int COMPOSITE_OUT = 39;
  const int COMPOSITE_OVER = 40;
  const int COMPOSITE_OVERLAY = 41;
  const int COMPOSITE_PLUS = 42;
  const int COMPOSITE_REPLACE = 43;
  const int COMPOSITE_SATURATE = 44;
  const int COMPOSITE_SCREEN = 45;
  const int COMPOSITE_SOFTLIGHT = 46;
  const int COMPOSITE_SRCATOP = 47;
  const int COMPOSITE_SRC = 48;
  const int COMPOSITE_SRCIN = 49;
  const int COMPOSITE_SRCOUT = 50;
  const int COMPOSITE_SRCOVER = 51;
  const int COMPOSITE_SUBTRACT = 52;
  const int COMPOSITE_THRESHOLD = 53;
  const int COMPOSITE_XOR = 54;
  const int COMPOSITE_CHANGEMASK = 6;
  const int COMPOSITE_LINEARLIGHT = 34;
  const int COMPOSITE_DIVIDE = 55;
  const int COMPOSITE_DISTORT = 56;
  const int COMPOSITE_BLUR = 57;
  const int COMPOSITE_PEGTOPLIGHT = 58;
  const int COMPOSITE_VIVIDLIGHT = 59;
  const int COMPOSITE_PINLIGHT = 60;
  const int COMPOSITE_LINEARDODGE = 61;
  const int COMPOSITE_LINEARBURN = 62;
  const int COMPOSITE_MATHEMATICS = 63;
  const int COMPOSITE_MODULUSADD = 2;
  const int COMPOSITE_MINUSDST = 36;
  const int COMPOSITE_DIVIDEDST = 55;
  const int COMPOSITE_DIVIDESRC = 64;
  const int COMPOSITE_MINUSSRC = 65;
  const int MONTAGEMODE_FRAME = 1;
  const int MONTAGEMODE_UNFRAME = 2;
  const int MONTAGEMODE_CONCATENATE = 3;
  const int STYLE_NORMAL = 1;
  const int STYLE_ITALIC = 2;
  const int STYLE_OBLIQUE = 3;
  const int STYLE_ANY = 4;
  const int FILTER_UNDEFINED = 0;
  const int FILTER_POINT = 1;
  const int FILTER_BOX = 2;
  const int FILTER_TRIANGLE = 3;
  const int FILTER_HERMITE = 4;
  const int FILTER_HANNING = 5;
  const int FILTER_HAMMING = 6;
  const int FILTER_BLACKMAN = 7;
  const int FILTER_GAUSSIAN = 8;
  const int FILTER_QUADRATIC = 9;
  const int FILTER_CUBIC = 10;
  const int FILTER_CATROM = 11;
  const int FILTER_MITCHELL = 12;
  const int FILTER_LANCZOS = 22;
  const int FILTER_BESSEL = 13;
  const int FILTER_SINC = 14;
  const int FILTER_KAISER = 16;
  const int FILTER_WELSH = 17;
  const int FILTER_PARZEN = 18;
  const int FILTER_LAGRANGE = 21;
  const int FILTER_SENTINEL = 27;
  const int FILTER_BOHMAN = 19;
  const int FILTER_BARTLETT = 20;
  const int FILTER_JINC = 13;
  const int FILTER_SINCFAST = 15;
  const int FILTER_ROBIDOUX = 26;
  const int FILTER_LANCZOSSHARP = 23;
  const int FILTER_LANCZOS2 = 24;
  const int FILTER_LANCZOS2SHARP = 25;
  const int IMGTYPE_UNDEFINED = 0;
  const int IMGTYPE_BILEVEL = 1;
  const int IMGTYPE_GRAYSCALE = 2;
  const int IMGTYPE_GRAYSCALEMATTE = 3;
  const int IMGTYPE_PALETTEMATTE = 5;
  const int IMGTYPE_TRUECOLOR = 6;
  const int IMGTYPE_TRUECOLORMATTE = 7;
  const int IMGTYPE_COLORSEPARATION = 8;
  const int IMGTYPE_OPTIMIZE = 10;
  const int IMGTYPE_PALETTE = 4;
  const int RESOLUTION_UNDEFINED = 0;
  const int COMPRESSION_UNDEFINED = 0;
  const int COMPRESSION_NO = 1;
  const int COMPRESSION_BZIP = 2;
  const int COMPRESSION_FAX = 6;
  const int COMPRESSION_GROUP4 = 7;
  const int COMPRESSION_JPEG = 8;
  const int COMPRESSION_JPEG2000 = 9;
  const int COMPRESSION_LZW = 11;
  const int COMPRESSION_RLE = 12;
  const int COMPRESSION_ZIP = 13;
  const int COMPRESSION_DXT1 = 3;
  const int COMPRESSION_DXT3 = 4;
  const int COMPRESSION_DXT5 = 5;
  const int COMPRESSION_ZIPS = 14;
  const int COMPRESSION_PIZ = 15;
  const int COMPRESSION_PXR24 = 16;
  const int COMPRESSION_B44 = 17;
  const int COMPRESSION_B44A = 18;
  const int COMPRESSION_LZMA = 19;
  const int COMPRESSION_JBIG1 = 20;
  const int COMPRESSION_JBIG2 = 21;
  const int PAINT_POINT = 1;
  const int PAINT_REPLACE = 2;
  const int PAINT_FLOODFILL = 3;
  const int PAINT_FILLTOBORDER = 4;
  const int PAINT_RESET = 5;
  const int GRAVITY_NORTHWEST = 1;
  const int GRAVITY_NORTH = 2;
  const int GRAVITY_NORTHEAST = 3;
  const int GRAVITY_WEST = 4;
  const int GRAVITY_CENTER = 5;
  const int GRAVITY_EAST = 6;
  const int GRAVITY_SOUTHWEST = 7;
  const int GRAVITY_SOUTH = 8;
  const int GRAVITY_SOUTHEAST = 9;
  const int STRETCH_NORMAL = 1;
  const int STRETCH_ULTRACONDENSED = 2;
  const int STRETCH_CONDENSED = 4;
  const int STRETCH_SEMICONDENSED = 5;
  const int STRETCH_SEMIEXPANDED = 6;
  const int STRETCH_EXPANDED = 7;
  const int STRETCH_EXTRAEXPANDED = 8;
  const int STRETCH_ULTRAEXPANDED = 9;
  const int STRETCH_ANY = 10;
  const int ALIGN_UNDEFINED = 0;
  const int ALIGN_LEFT = 1;
  const int ALIGN_CENTER = 2;
  const int ALIGN_RIGHT = 3;
  const int DECORATION_NO = 1;
  const int DECORATION_UNDERLINE = 2;
  const int DECORATION_OVERLINE = 3;
  const int DECORATION_LINETROUGH = 4;
  const int NOISE_UNIFORM = 1;
  const int NOISE_GAUSSIAN = 2;
  const int NOISE_IMPULSE = 4;
  const int NOISE_LAPLACIAN = 5;
  const int NOISE_POISSON = 6;
  const int NOISE_RANDOM = 7;
  const int CHANNEL_UNDEFINED = 0;
  const int CHANNEL_RED = 1;
  const int CHANNEL_GRAY = 1;
  const int CHANNEL_CYAN = 1;
  const int CHANNEL_GREEN = 2;
  const int CHANNEL_MAGENTA = 2;
  const int CHANNEL_BLUE = 4;
  const int CHANNEL_YELLOW = 4;
  const int CHANNEL_ALPHA = 8;
  const int CHANNEL_OPACITY = 8;
  const int CHANNEL_MATTE = 8;
  const int CHANNEL_BLACK = 32;
  const int CHANNEL_INDEX = 32;
  const int CHANNEL_ALL = -1;
  const int CHANNEL_DEFAULT = -9;
  const int CHANNEL_TRUEALPHA = 64;
  const int CHANNEL_RGBS = 128;
  const int CHANNEL_SYNC = 256;
  const int CHANNEL_COMPOSITES = 47;
  const int METRIC_UNDEFINED = 0;
  const int METRIC_MEANSQUAREERROR = 4;
  const int PIXEL_CHAR = 1;
  const int PIXEL_DOUBLE = 2;
  const int PIXEL_FLOAT = 3;
  const int PIXEL_INTEGER = 4;
  const int PIXEL_LONG = 5;
  const int PIXEL_QUANTUM = 6;
  const int PIXEL_SHORT = 7;
  const int EVALUATE_UNDEFINED = 0;
  const int EVALUATE_ADD = 1;
  const int EVALUATE_AND = 2;
  const int EVALUATE_DIVIDE = 3;
  const int EVALUATE_LEFTSHIFT = 4;
  const int EVALUATE_MAX = 5;
  const int EVALUATE_MIN = 6;
  const int EVALUATE_MULTIPLY = 7;
  const int EVALUATE_OR = 8;
  const int EVALUATE_RIGHTSHIFT = 9;
  const int EVALUATE_SET = 10;
  const int EVALUATE_SUBTRACT = 11;
  const int EVALUATE_XOR = 12;
  const int EVALUATE_POW = 13;
  const int EVALUATE_LOG = 14;
  const int EVALUATE_THRESHOLD = 15;
  const int EVALUATE_THRESHOLDBLACK = 16;
  const int EVALUATE_THRESHOLDWHITE = 17;
  const int EVALUATE_GAUSSIANNOISE = 18;
  const int EVALUATE_IMPULSENOISE = 19;
  const int EVALUATE_LAPLACIANNOISE = 20;
  const int EVALUATE_POISSONNOISE = 22;
  const int EVALUATE_UNIFORMNOISE = 23;
  const int EVALUATE_COSINE = 24;
  const int EVALUATE_SINE = 25;
  const int EVALUATE_ADDMODULUS = 26;
  const int EVALUATE_MEAN = 27;
  const int EVALUATE_ABS = 28;
  const int EVALUATE_EXPONENTIAL = 29;
  const int EVALUATE_MEDIAN = 30;
  const int COLORSPACE_UNDEFINED = 0;
  const int COLORSPACE_RGB = 1;
  const int COLORSPACE_GRAY = 2;
  const int COLORSPACE_TRANSPARENT = 3;
  const int COLORSPACE_OHTA = 4;
  const int COLORSPACE_LAB = 5;
  const int COLORSPACE_XYZ = 6;
  const int COLORSPACE_YCBCR = 7;
  const int COLORSPACE_YCC = 8;
  const int COLORSPACE_YIQ = 9;
  const int COLORSPACE_YPBPR = 10;
  const int COLORSPACE_YUV = 11;
  const int COLORSPACE_CMYK = 12;
  const int COLORSPACE_SRGB = 13;
  const int COLORSPACE_HSB = 14;
  const int COLORSPACE_HSL = 15;
  const int COLORSPACE_HWB = 16;
  const int COLORSPACE_REC601LUMA = 17;
  const int COLORSPACE_REC709LUMA = 19;
  const int COLORSPACE_LOG = 21;
  const int COLORSPACE_CMY = 22;
  const int VIRTUALPIXELMETHOD_EDGE = 4;
  const int VIRTUALPIXELMETHOD_TILE = 7;
  const int VIRTUALPIXELMETHOD_MASK = 9;
  const int VIRTUALPIXELMETHOD_GRAY = 11;
  const int PREVIEW_UNDEFINED = 0;
  const int PREVIEW_ROTATE = 1;
  const int PREVIEW_SHEAR = 2;
  const int PREVIEW_ROLL = 3;
  const int PREVIEW_HUE = 4;
  const int PREVIEW_SATURATION = 5;
  const int PREVIEW_BRIGHTNESS = 6;
  const int PREVIEW_GAMMA = 7;
  const int PREVIEW_SPIFF = 8;
  const int PREVIEW_DULL = 9;
  const int PREVIEW_GRAYSCALE = 10;
  const int PREVIEW_QUANTIZE = 11;
  const int PREVIEW_DESPECKLE = 12;
  const int PREVIEW_REDUCENOISE = 13;
  const int PREVIEW_ADDNOISE = 14;
  const int PREVIEW_SHARPEN = 15;
  const int PREVIEW_BLUR = 16;
  const int PREVIEW_THRESHOLD = 17;
  const int PREVIEW_EDGEDETECT = 18;
  const int PREVIEW_SPREAD = 19;
  const int PREVIEW_SOLARIZE = 20;
  const int PREVIEW_SHADE = 21;
  const int PREVIEW_RAISE = 22;
  const int PREVIEW_SEGMENT = 23;
  const int PREVIEW_SWIRL = 24;
  const int PREVIEW_IMPLODE = 25;
  const int PREVIEW_WAVE = 26;
  const int PREVIEW_OILPAINT = 27;
  const int PREVIEW_CHARCOALDRAWING = 28;
  const int PREVIEW_JPEG = 29;
  const int INTERLACE_UNDEFINED = 0;
  const int INTERLACE_NO = 1;
  const int INTERLACE_LINE = 2;
  const int INTERLACE_PLANE = 3;
  const int INTERLACE_PARTITION = 4;
  const int INTERLACE_GIF = 5;
  const int INTERLACE_JPEG = 6;
  const int INTERLACE_PNG = 7;
  const int FILLRULE_UNDEFINED = 0;
  const int FILLRULE_EVENODD = 1;
  const int FILLRULE_NONZERO = 2;
  const int PATHUNITS_UNDEFINED = 0;
  const int PATHUNITS_USERSPACE = 1;
  const int LINECAP_UNDEFINED = 0;
  const int LINECAP_BUTT = 1;
  const int LINECAP_ROUND = 2;
  const int LINECAP_SQUARE = 3;
  const int LINEJOIN_UNDEFINED = 0;
  const int LINEJOIN_MITER = 1;
  const int LINEJOIN_ROUND = 2;
  const int LINEJOIN_BEVEL = 3;
  const int RESOURCETYPE_UNDEFINED = 0;
  const int RESOURCETYPE_AREA = 1;
  const int RESOURCETYPE_DISK = 2;
  const int RESOURCETYPE_FILE = 3;
  const int RESOURCETYPE_MAP = 4;
  const int RESOURCETYPE_MEMORY = 5;
  const int LAYERMETHOD_UNDEFINED = 0;
  const int LAYERMETHOD_COALESCE = 1;
  const int LAYERMETHOD_COMPAREANY = 2;
  const int LAYERMETHOD_DISPOSE = 5;
  const int LAYERMETHOD_OPTIMIZE = 6;
  const int LAYERMETHOD_COMPOSITE = 12;
  const int LAYERMETHOD_REMOVEDUPS = 10;
  const int LAYERMETHOD_REMOVEZERO = 11;
  const int LAYERMETHOD_MERGE = 13;
  const int LAYERMETHOD_FLATTEN = 14;
  const int LAYERMETHOD_MOSAIC = 15;
  const int LAYERMETHOD_TRIMBOUNDS = 16;
  const int ORIENTATION_UNDEFINED = 0;
  const int ORIENTATION_TOPLEFT = 1;
  const int ORIENTATION_TOPRIGHT = 2;
  const int ORIENTATION_BOTTOMRIGHT = 3;
  const int ORIENTATION_BOTTOMLEFT = 4;
  const int ORIENTATION_LEFTTOP = 5;
  const int ORIENTATION_RIGHTTOP = 6;
  const int ORIENTATION_RIGHTBOTTOM = 7;
  const int ORIENTATION_LEFTBOTTOM = 8;
  const int DISTORTION_UNDEFINED = 0;
  const int DISTORTION_AFFINE = 1;
  const int DISTORTION_ARC = 9;
  const int DISTORTION_BILINEAR = 6;
  const int DISTORTION_PERSPECTIVE = 4;
  const int DISTORTION_POLYNOMIAL = 8;
  const int DISTORTION_POLAR = 10;
  const int DISTORTION_DEPOLAR = 11;
  const int DISTORTION_BARREL = 14;
  const int DISTORTION_SHEPARDS = 16;
  const int DISTORTION_SENTINEL = 18;
  const int DISTORTION_RESIZE = 17;
  const int ALPHACHANNEL_ACTIVATE = 1;
  const int ALPHACHANNEL_DEACTIVATE = 4;
  const int ALPHACHANNEL_RESET = 7;
  const int ALPHACHANNEL_SET = 8;
  const int ALPHACHANNEL_UNDEFINED = 0;
  const int ALPHACHANNEL_COPY = 3;
  const int ALPHACHANNEL_EXTRACT = 5;
  const int ALPHACHANNEL_OPAQUE = 6;
  const int ALPHACHANNEL_SHAPE = 9;
  const int FUNCTION_UNDEFINED = 0;
  const int FUNCTION_POLYNOMIAL = 1;
  const int FUNCTION_SINUSOID = 2;
  const int FUNCTION_ARCSIN = 3;
  const int FUNCTION_ARCTAN = 4;
  const int INTERPOLATE_UNDEFINED = 0;
  const int INTERPOLATE_AVERAGE = 1;
  const int INTERPOLATE_BICUBIC = 2;
  const int INTERPOLATE_BILINEAR = 3;
  const int INTERPOLATE_FILTER = 4;
  const int INTERPOLATE_INTEGER = 5;
  const int INTERPOLATE_MESH = 6;
  const int INTERPOLATE_SPLINE = 8;
  const int DITHERMETHOD_UNDEFINED = 0;
  const int DITHERMETHOD_NO = 1;
  const int DITHERMETHOD_RIEMERSMA = 2;

  // Methods
  public function count(): int;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function adaptiveBlurImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function adaptiveResizeImage(
    int $columns,
    int $rows,
    bool $bestfit = false,
  ): bool;
  public function adaptiveSharpenImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function adaptiveThresholdImage(
    int $width,
    int $height,
    int $offset,
  ): bool;
  public function addImage(Imagick $source): bool;
  public function addNoiseImage(
    int $noise_type,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function affineTransformImage(ImagickDraw $matrix): bool;
  public function animateImages(string $x_server): bool;
  public function annotateImage(
    ImagickDraw $draw_settings,
    float $x,
    float $y,
    float $angle,
    string $text,
  ): bool;
  public function appendImages(bool $stack = false): Imagick;
  public function averageImages(): Imagick;
  public function blackThresholdImage($threshold): bool;
  public function blurImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function borderImage($bordercolor, int $width, int $height): bool;
  public function charcoalImage(float $radius, float $sigma): bool;
  public function chopImage(int $width, int $height, int $x, int $y): bool;
  public function clear(): bool;
  public function clipImage(): bool;
  public function clipPathImage(string $pathname, bool $inside): bool;
  public function __clone(): void;
  public function clutImage(
    Imagick $lookup_table,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function coalesceImages(): Imagick;
  public function colorFloodfillImage(
    $fill,
    float $fuzz,
    $bordercolor,
    int $x,
    int $y,
  ): bool;
  public function colorizeImage($colorize, $opacity): bool;
  public function combineImages(int $channelType): Imagick;
  public function commentImage(string $comment): bool;
  public function compareImageChannels(
    Imagick $image,
    int $channelType,
    int $metricType,
  ): varray;
  public function compareImageLayers(int $method): Imagick;
  public function compareImages(Imagick $compare, int $metric): varray;
  public function compositeImage(
    Imagick $composite_object,
    int $composite,
    int $x,
    int $y,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function __construct($files = null);
  public function contrastImage(bool $sharpen): bool;
  public function contrastStretchImage(
    float $black_point,
    float $white_point,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function convolveImage(
    varray $kernel,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function cropImage(int $width, int $height, int $x, int $y): bool;
  public function cropThumbnailImage(int $width, int $height): bool;
  public function current(): Imagick;
  public function cycleColormapImage(int $displace): bool;
  public function decipherImage(string $passphrase): bool;
  public function deconstructImages(): Imagick;
  public function deleteImageArtifact(string $artifact): bool;
  public function deskewImage(float $threshold): bool;
  public function despeckleImage(): bool;
  public function destroy(): bool;
  public function displayImage(string $servername): bool;
  public function displayImages(string $servername): bool;
  public function distortImage(
    int $method,
    varray $arguments,
    bool $bestfit,
  ): bool;
  public function drawImage(ImagickDraw $draw): bool;
  public function edgeImage(float $radius): bool;
  public function embossImage(float $radius, float $sigma): bool;
  public function encipherImage(string $passphrase): bool;
  public function enhanceImage(): bool;
  public function equalizeImage(): bool;
  public function evaluateImage(
    int $op,
    float $constant,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function exportImagePixels(
    int $x,
    int $y,
    int $width,
    int $height,
    string $map,
    int $storage,
  ): varray;
  public function extentImage(int $width, int $height, int $x, int $y): bool;
  public function flattenImages(): Imagick;
  public function flipImage(): bool;
  public function floodFillPaintImage(
    $fill,
    float $fuzz,
    $target,
    int $x,
    int $y,
    bool $invert,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function flopImage(): bool;
  public function frameImage(
    $matte_color,
    int $width,
    int $height,
    int $inner_bevel,
    int $outer_bevel,
  ): bool;
  public function functionImage(
    int $function,
    varray $arguments,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function fxImage(
    string $expression,
    int $channel = \Imagick::CHANNEL_ALL,
  ): Imagick;
  public function gammaImage(
    float $gamma,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function gaussianBlurImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function getColorspace(): int;
  public function getCompression(): int;
  public function getCompressionQuality(): int;
  public static function getCopyright(): string;
  public function getFilename(): string;
  public function getFont(): string;
  public function getFormat(): string;
  public function getGravity(): int;
  public static function getHomeURL(): string;
  public function getImage(): Imagick;
  public function getImageAlphaChannel(): int;
  public function getImageArtifact(string $artifact): string;
  public function getImageBackgroundColor(): ImagickPixel;
  public function getImageBlob(): string;
  public function getImageBluePrimary(): darray;
  public function getImageBorderColor(): ImagickPixel;
  public function getImageChannelDepth(int $channel): int;
  public function getImageChannelDistortion(
    Imagick $reference,
    int $channel,
    int $metric,
  ): float;
  public function getImageChannelDistortions(
    Imagick $reference,
    int $metric,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): float;
  public function getImageChannelExtrema(int $channel): darray;
  public function getImageChannelKurtosis(
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): darray;
  public function getImageChannelMean(int $channel): darray;
  public function getImageChannelRange(int $channel): darray;
  public function getImageChannelStatistics(): darray;
  public function getImageClipMask(): Imagick;
  public function getImageColormapColor(int $index): ImagickPixel;
  public function getImageColors(): int;
  public function getImageColorspace(): int;
  public function getImageCompose(): int;
  public function getImageCompression(): int;
  public function getImageDelay(): int;
  public function getImageDepth(): int;
  public function getImageDispose(): int;
  /** HHVM doesn't support MagickWand:
  public function getImageDistortion(
    MagickWand $reference,
    int $metric,
  ): float;
  */
  public function getImageExtrema(): darray;
  public function getImageFilename(): string;
  public function getImageFormat(): string;
  public function getImageGamma(): float;
  public function getImageGeometry(): darray;
  public function getImageGravity(): int;
  public function getImageGreenPrimary(): darray;
  public function getImageHeight(): int;
  public function getImageHistogram(): varray;
  public function getImageIndex(): int;
  public function getImageInterlaceScheme(): int;
  public function getImageInterpolateMethod(): int;
  public function getImageIterations(): int;
  public function getImageLength(): int;
  public function getImageMatte(): bool;
  public function getImageMatteColor(): ImagickPixel;
  public function getImageMimeType(): string;
  public function getImageOrientation(): int;
  public function getImagePage(): darray;
  public function getImagePixelColor(int $x, int $y): ImagickPixel;
  public function getImageProfile(string $name): string;
  public function getImageProfiles(
    string $pattern = "*",
    bool $with_values = true,
  ): varray_or_darray;
  public function getImageProperties(
    string $pattern = "*",
    bool $with_values = true,
  ): varray_or_darray;
  public function getImageProperty(string $name): string;
  public function getImageRedPrimary(): darray;
  public function getImageRegion(
    int $width,
    int $height,
    int $x,
    int $y,
  ): Imagick;
  public function getImageRenderingIntent(): int;
  public function getImageResolution(): darray;
  public function getImagesBlob(): string;
  public function getImageScene(): int;
  public function getImageSignature(): string;
  public function getImageSize(): int;
  public function getImageTicksPerSecond(): int;
  public function getImageTotalInkDensity(): float;
  public function getImageType(): int;
  public function getImageUnits(): int;
  public function getImageVirtualPixelMethod(): int;
  public function getImageWhitePoint(): darray;
  public function getImageWidth(): int;
  public function getInterlaceScheme(): int;
  public function getIteratorIndex(): int;
  public function getNumberImages(): int;
  public function getOption(string $key): string;
  public static function getPackageName(): string;
  public function getPage(): darray;
  public function getPixelIterator(): ImagickPixelIterator;
  public function getPixelRegionIterator(
    int $x,
    int $y,
    int $columns,
    int $rows,
  ): ImagickPixelIterator;
  public function getPointSize(): float;
  public static function getQuantumDepth(): darray;
  public static function getQuantumRange(): darray;
  public static function getReleaseDate(): string;
  public static function getResource(int $type): int;
  public static function getResourceLimit(int $type): int;
  public function getSamplingFactors(): varray;
  public function getSize(): darray;
  public function getSizeOffset(): int;
  public static function getVersion(): darray;
  public function haldClutImage(
    Imagick $clut,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function hasNextImage(): bool;
  public function hasPreviousImage(): bool;
  public function identifyImage(bool $appendRawOutput = false): darray;
  public function implodeImage(float $radius): bool;
  public function importImagePixels(
    int $x,
    int $y,
    int $width,
    int $height,
    string $map,
    int $storage,
    varray $pixels,
  ): bool;
  public function labelImage(string $label): bool;
  public function levelImage(
    float $blackPoint,
    float $gamma,
    float $whitePoint,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function linearStretchImage(
    float $blackPoint,
    float $whitePoint,
  ): bool;
  public function liquidRescaleImage(
    int $width,
    int $height,
    float $delta_x,
    float $rigidity,
  ): bool;
  public function magnifyImage(): bool;
  public function mapImage(Imagick $map, bool $dither): bool;
  public function matteFloodfillImage(
    float $alpha,
    float $fuzz,
    $bordercolor,
    int $x,
    int $y,
  ): bool;
  public function medianFilterImage(float $radius): bool;
  public function mergeImageLayers(int $layer_method): Imagick;
  public function minifyImage(): bool;
  public function modulateImage(
    float $brightness,
    float $saturation,
    float $hue,
  ): bool;
  public function montageImage(
    ImagickDraw $draw,
    string $tile_geometry,
    string $thumbnail_geometry,
    int $mode,
    string $frame,
  ): Imagick;
  public function morphImages(int $number_frames): Imagick;
  public function mosaicImages(): Imagick;
  public function motionBlurImage(
    float $radius,
    float $sigma,
    float $angle,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function negateImage(
    bool $gray,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function newImage(
    int $cols,
    int $rows,
    $background,
    string $format = "",
  ): bool;
  public function newPseudoImage(
    int $columns,
    int $rows,
    string $pseudoString,
  ): bool;
  public function nextImage(): bool;
  public function normalizeImage(int $channel = \Imagick::CHANNEL_ALL): bool;
  public function oilPaintImage(float $radius): bool;
  public function opaquePaintImage(
    $target,
    $fill,
    float $fuzz,
    bool $invert,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function optimizeImageLayers(): Imagick;
  public function orderedPosterizeImage(
    string $threshold_map,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function paintFloodfillImage(
    $fill,
    float $fuzz,
    $bordercolor,
    int $x,
    int $y,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function paintOpaqueImage(
    $target,
    $fill,
    float $fuzz,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function paintTransparentImage(
    $target,
    float $alpha,
    float $fuzz,
  ): bool;
  public function pingImage(string $filename): bool;
  public function pingImageBlob(string $image): bool;
  public function pingImageFile(resource $filehandle, string $fileName): bool;
  public function polaroidImage(ImagickDraw $properties, float $angle): bool;
  public function posterizeImage(int $levels, bool $dither): bool;
  public function previewImages(int $preview): Imagick;
  public function previousImage(): bool;
  public function profileImage(string $name, string $profile): bool;
  public function quantizeImage(
    int $numberColors,
    int $colorspace,
    int $treedepth,
    bool $dither,
    bool $measureError,
  ): bool;
  public function quantizeImages(
    int $numberColors,
    int $colorspace,
    int $treedepth,
    bool $dither,
    bool $measureError,
  ): bool;
  public function queryFontMetrics(
    ImagickDraw $properties,
    string $text,
    $multiline = null,
  ): darray;
  public static function queryFonts(string $pattern = "*"): varray;
  public static function queryFormats(string $pattern = "*"): varray;
  public function radialBlurImage(
    float $angle,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function raiseImage(
    int $width,
    int $height,
    int $x,
    int $y,
    bool $raise,
  ): bool;
  public function randomThresholdImage(
    float $low,
    float $high,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function readImage(string $filename): bool;
  public function readImageBlob(string $image, string $filename = ""): bool;
  public function readImageFile(
    resource $filehandle,
    string $fileName = "",
  ): bool;
  public function readImages(varray $files): bool;
  public function recolorImage(varray $matrix): bool;
  public function reduceNoiseImage(float $radius): bool;
  public function remapImage(Imagick $replacement, int $dither): bool;
  public function removeImage(): bool;
  public function removeImageProfile(string $name): string;
  public function resampleImage(
    float $x_resolution,
    float $y_resolution,
    int $filter,
    float $blur,
  ): bool;
  public function resetImagePage(string $page): bool;
  public function resizeImage(
    int $columns,
    int $rows,
    int $filter,
    float $blur,
    bool $bestfit = false,
  ): bool;
  public function rollImage(int $x, int $y): bool;
  public function rotateImage($background, float $degrees): bool;
  public function roundCorners(
    float $x_rounding,
    float $y_rounding,
    float $stroke_width = 10.0,
    float $displace = 5.0,
    float $size_correction = -6.0,
  ): bool;
  public function roundCornersImage(
    float $x_rounding,
    float $y_rounding,
    float $stroke_width = 10.0,
    float $displace = 5.0,
    float $size_correction = -6.0,
  ): bool;
  public function sampleImage(int $columns, int $rows): bool;
  public function scaleImage(
    int $cols,
    int $rows,
    bool $bestfit = false,
  ): bool;
  public function segmentImage(
    int $COLORSPACE,
    float $cluster_threshold,
    float $smooth_threshold,
    bool $verbose = false,
  ): bool;
  public function separateImageChannel(int $channel): bool;
  public function sepiaToneImage(float $threshold): bool;
  public function setBackgroundColor($background): bool;
  public function setColorspace(int $COLORSPACE): bool;
  public function setCompression(int $compression): bool;
  public function setCompressionQuality(int $quality): bool;
  public function setFilename(string $filename): bool;
  public function setFirstIterator(): bool;
  public function setFont(string $font): bool;
  public function setFormat(string $format): bool;
  public function setGravity(int $gravity): bool;
  public function setImage(Imagick $replace): bool;
  public function setImageAlphaChannel(int $mode): bool;
  public function setImageArtifact(string $artifact, string $value): bool;
  public function setImageBackgroundColor($background): bool;
  public function setImageBias(float $bias): bool;
  public function setImageBluePrimary(float $x, float $y): bool;
  public function setImageBorderColor($border): bool;
  public function setImageChannelDepth(int $channel, int $depth): bool;
  public function setImageClipMask(Imagick $clip_mask): bool;
  public function setImageColormapColor(int $index, $color): bool;
  public function setImageColorspace(int $colorspace): bool;
  public function setImageCompose(int $compose): bool;
  public function setImageCompression(int $compression): bool;
  public function setImageCompressionQuality(int $quality): bool;
  public function setImageDelay(int $delay): bool;
  public function setImageDepth(int $depth): bool;
  public function setImageDispose(int $dispose): bool;
  public function setImageExtent(int $columns, int $rows): bool;
  public function setImageFilename(string $filename): bool;
  public function setImageFormat(string $format): bool;
  public function setImageGamma(float $gamma): bool;
  public function setImageGravity(int $gravity): bool;
  public function setImageGreenPrimary(float $x, float $y): bool;
  public function setImageIndex(int $index): bool;
  public function setImageInterlaceScheme(int $interlace_scheme): bool;
  public function setImageInterpolateMethod(int $method): bool;
  public function setImageIterations(int $iterations): bool;
  public function setImageMatte(bool $matte): bool;
  public function setImageMatteColor($matte): bool;
  public function setImageOpacity(float $opacity): bool;
  public function setImageOrientation(int $orientation): bool;
  public function setImagePage(int $width, int $height, int $x, int $y): bool;
  public function setImageProfile(string $name, string $profile): bool;
  public function setImageProperty(string $name, string $value): bool;
  public function setImageRedPrimary(float $x, float $y): bool;
  public function setImageRenderingIntent(int $rendering_intent): bool;
  public function setImageResolution(
    float $x_resolution,
    float $y_resolution,
  ): bool;
  public function setImageScene(int $scene): bool;
  public function setImageTicksPerSecond(int $ticks_per_second): bool;
  public function setImageType(int $image_type): bool;
  public function setImageUnits(int $units): bool;
  public function setImageVirtualPixelMethod(int $method): bool;
  public function setImageWhitePoint(float $x, float $y): bool;
  public function setInterlaceScheme(int $interlace_scheme): bool;
  public function setIteratorIndex(int $index): bool;
  public function setLastIterator(): bool;
  public function setOption(string $key, string $value): bool;
  public function setPage(int $width, int $height, int $x, int $y): bool;
  public function setPointSize(float $point_size): bool;
  public function setResolution(
    float $x_resolution,
    float $y_resolution,
  ): bool;
  public static function setResourceLimit(int $type, int $limit): bool;
  public function setSamplingFactors(varray $factors): bool;
  public function setSize(int $columns, int $rows): bool;
  public function setSizeOffset(int $columns, int $rows, int $offset): bool;
  public function setType(int $image_type): bool;
  public function shadeImage(
    bool $gray,
    float $azimuth,
    float $elevation,
  ): bool;
  public function shadowImage(
    float $opacity,
    float $sigma,
    int $x,
    int $y,
  ): bool;
  public function sharpenImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function shaveImage(int $columns, int $rows): bool;
  public function shearImage(
    $background,
    float $x_shear,
    float $y_shear,
  ): bool;
  public function sigmoidalContrastImage(
    bool $sharpen,
    float $alpha,
    float $beta,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function sketchImage(
    float $radius,
    float $sigma,
    float $angle,
  ): bool;
  public function solarizeImage(int $threshold): bool;
  public function sparseColorImage(
    int $SPARSE_METHOD,
    varray $arguments,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function spliceImage(int $width, int $height, int $x, int $y): bool;
  public function spreadImage(float $radius): bool;
  public function steganoImage(Imagick $watermark_wand, int $offset): Imagick;
  public function stereoImage(Imagick $offset_wand): Imagick;
  public function stripImage(): bool;
  public function swirlImage(float $degrees): bool;
  public function textureImage(Imagick $texture_wand): Imagick;
  public function thresholdImage(
    float $threshold,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function thumbnailImage(
    int $columns,
    int $rows,
    bool $bestfit = false,
    bool $fill = false,
  ): bool;
  public function tintImage($tint, $opacity): bool;
  public function transformImage(string $crop, string $geometry): Imagick;
  public function transparentPaintImage(
    $target,
    float $alpha,
    float $fuzz,
    bool $invert,
  ): bool;
  public function transposeImage(): bool;
  public function transverseImage(): bool;
  public function trimImage(float $fuzz): bool;
  public function uniqueImageColors(): bool;
  public function unsharpMaskImage(
    float $radius,
    float $sigma,
    float $amount,
    float $threshold,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function valid(): bool;
  public function vignetteImage(
    float $blackPoint,
    float $whitePoint,
    int $x,
    int $y,
  ): bool;
  public function waveImage(float $amplitude, float $length): bool;
  public function whiteThresholdImage($threshold): bool;
  public function writeImage(string $filename = ""): bool;
  public function writeImageFile(
    resource $filehandle,
    string $format = "",
  ): bool;
  public function writeImages(string $filename, bool $adjoin): bool;
  public function writeImagesFile(
    resource $filehandle,
    string $format = "",
  ): bool;
}
