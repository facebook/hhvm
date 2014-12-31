<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Imagick
  implements Countable, Iterator<Imagick>, Traversable<Imagick> {

  // Constants
  const COMPOSITE_MODULUSSUBTRACT = 52;
  const COMPOSITE_DARKENINTENSITY = 66;
  const COMPOSITE_LIGHTENINTENSITY = 67;
  const IMGTYPE_COLORSEPARATIONMATTE = 9;
  const IMGTYPE_PALETTEBILEVELMATTE = 11;
  const RESOLUTION_PIXELSPERINCH = 1;
  const RESOLUTION_PIXELSPERCENTIMETER = 2;
  const COMPRESSION_LOSSLESSJPEG = 10;
  const NOISE_MULTIPLICATIVEGAUSSIAN = 3;
  const METRIC_MEANABSOLUTEERROR = 2;
  const METRIC_PEAKABSOLUTEERROR = 5;
  const METRIC_PEAKSIGNALTONOISERATIO = 6;
  const METRIC_ROOTMEANSQUAREDERROR = 7;
  const EVALUATE_MULTIPLICATIVENOISE = 21;
  const VIRTUALPIXELMETHOD_UNDEFINED = 0;
  const VIRTUALPIXELMETHOD_BACKGROUND = 1;
  const VIRTUALPIXELMETHOD_CONSTANT = 2;
  const VIRTUALPIXELMETHOD_MIRROR = 5;
  const VIRTUALPIXELMETHOD_TRANSPARENT = 8;
  const VIRTUALPIXELMETHOD_BLACK = 10;
  const VIRTUALPIXELMETHOD_WHITE = 12;
  const VIRTUALPIXELMETHOD_HORIZONTALTILE = 13;
  const VIRTUALPIXELMETHOD_VERTICALTILE = 14;
  const VIRTUALPIXELMETHOD_HORIZONTALTILEEDGE = 15;
  const VIRTUALPIXELMETHOD_VERTICALTILEEDGE = 16;
  const VIRTUALPIXELMETHOD_CHECKERTILE = 17;
  const RENDERINGINTENT_UNDEFINED = 0;
  const RENDERINGINTENT_SATURATION = 1;
  const RENDERINGINTENT_PERCEPTUAL = 2;
  const RENDERINGINTENT_ABSOLUTE = 3;
  const RENDERINGINTENT_RELATIVE = 4;
  const PATHUNITS_USERSPACEONUSE = 2;
  const PATHUNITS_OBJECTBOUNDINGBOX = 3;
  const LAYERMETHOD_COMPARECLEAR = 3;
  const LAYERMETHOD_COMPAREOVERLAY = 4;
  const LAYERMETHOD_OPTIMIZEPLUS = 8;
  const LAYERMETHOD_OPTIMIZETRANS = 9;
  const LAYERMETHOD_OPTIMIZEIMAGE = 7;
  const DISTORTION_AFFINEPROJECTION = 2;
  const DISTORTION_PERSPECTIVEPROJECTION = 5;
  const DISTORTION_SCALEROTATETRANSLATE = 3;
  const DISTORTION_BARRELINVERSE = 15;
  const DISTORTION_BILINEARFORWARD = 6;
  const DISTORTION_BILINEARREVERSE = 7;
  const DISTORTION_CYLINDER2PLANE = 12;
  const DISTORTION_PLANE2CYLINDER = 13;
  const ALPHACHANNEL_TRANSPARENT = 10;
  const SPARSECOLORMETHOD_UNDEFINED = 0;
  const SPARSECOLORMETHOD_BARYCENTRIC = 1;
  const SPARSECOLORMETHOD_BILINEAR = 7;
  const SPARSECOLORMETHOD_POLYNOMIAL = 8;
  const SPARSECOLORMETHOD_SPEPARDS = 16;
  const SPARSECOLORMETHOD_VORONOI = 18;
  const INTERPOLATE_NEARESTNEIGHBOR = 7;
  const DITHERMETHOD_FLOYDSTEINBERG = 3;
  const COLOR_BLACK = 0;
  const COLOR_BLUE = 1;
  const COLOR_CYAN = 2;
  const COLOR_GREEN = 3;
  const COLOR_RED = 4;
  const COLOR_YELLOW = 5;
  const COLOR_MAGENTA = 6;
  const COLOR_OPACITY = 7;
  const COLOR_ALPHA = 8;
  const COLOR_FUZZ = 9;
  const DISPOSE_UNRECOGNIZED = 0;
  const DISPOSE_UNDEFINED = 0;
  const DISPOSE_NONE = 1;
  const DISPOSE_BACKGROUND = 2;
  const DISPOSE_PREVIOUS = 3;
  const COMPOSITE_DEFAULT = 40;
  const COMPOSITE_UNDEFINED = 0;
  const COMPOSITE_NO = 1;
  const COMPOSITE_ADD = 2;
  const COMPOSITE_ATOP = 3;
  const COMPOSITE_BLEND = 4;
  const COMPOSITE_BUMPMAP = 5;
  const COMPOSITE_CLEAR = 7;
  const COMPOSITE_COLORBURN = 8;
  const COMPOSITE_COLORDODGE = 9;
  const COMPOSITE_COLORIZE = 10;
  const COMPOSITE_COPYBLACK = 11;
  const COMPOSITE_COPYBLUE = 12;
  const COMPOSITE_COPY = 13;
  const COMPOSITE_COPYCYAN = 14;
  const COMPOSITE_COPYGREEN = 15;
  const COMPOSITE_COPYMAGENTA = 16;
  const COMPOSITE_COPYOPACITY = 17;
  const COMPOSITE_COPYRED = 18;
  const COMPOSITE_COPYYELLOW = 19;
  const COMPOSITE_DARKEN = 20;
  const COMPOSITE_DSTATOP = 21;
  const COMPOSITE_DST = 22;
  const COMPOSITE_DSTIN = 23;
  const COMPOSITE_DSTOUT = 24;
  const COMPOSITE_DSTOVER = 25;
  const COMPOSITE_DIFFERENCE = 26;
  const COMPOSITE_DISPLACE = 27;
  const COMPOSITE_DISSOLVE = 28;
  const COMPOSITE_EXCLUSION = 29;
  const COMPOSITE_HARDLIGHT = 30;
  const COMPOSITE_HUE = 31;
  const COMPOSITE_IN = 32;
  const COMPOSITE_LIGHTEN = 33;
  const COMPOSITE_LUMINIZE = 35;
  const COMPOSITE_MINUS = 36;
  const COMPOSITE_MODULATE = 37;
  const COMPOSITE_MULTIPLY = 38;
  const COMPOSITE_OUT = 39;
  const COMPOSITE_OVER = 40;
  const COMPOSITE_OVERLAY = 41;
  const COMPOSITE_PLUS = 42;
  const COMPOSITE_REPLACE = 43;
  const COMPOSITE_SATURATE = 44;
  const COMPOSITE_SCREEN = 45;
  const COMPOSITE_SOFTLIGHT = 46;
  const COMPOSITE_SRCATOP = 47;
  const COMPOSITE_SRC = 48;
  const COMPOSITE_SRCIN = 49;
  const COMPOSITE_SRCOUT = 50;
  const COMPOSITE_SRCOVER = 51;
  const COMPOSITE_SUBTRACT = 52;
  const COMPOSITE_THRESHOLD = 53;
  const COMPOSITE_XOR = 54;
  const COMPOSITE_CHANGEMASK = 6;
  const COMPOSITE_LINEARLIGHT = 34;
  const COMPOSITE_DIVIDE = 55;
  const COMPOSITE_DISTORT = 56;
  const COMPOSITE_BLUR = 57;
  const COMPOSITE_PEGTOPLIGHT = 58;
  const COMPOSITE_VIVIDLIGHT = 59;
  const COMPOSITE_PINLIGHT = 60;
  const COMPOSITE_LINEARDODGE = 61;
  const COMPOSITE_LINEARBURN = 62;
  const COMPOSITE_MATHEMATICS = 63;
  const COMPOSITE_MODULUSADD = 2;
  const COMPOSITE_MINUSDST = 36;
  const COMPOSITE_DIVIDEDST = 55;
  const COMPOSITE_DIVIDESRC = 64;
  const COMPOSITE_MINUSSRC = 65;
  const MONTAGEMODE_FRAME = 1;
  const MONTAGEMODE_UNFRAME = 2;
  const MONTAGEMODE_CONCATENATE = 3;
  const STYLE_NORMAL = 1;
  const STYLE_ITALIC = 2;
  const STYLE_OBLIQUE = 3;
  const STYLE_ANY = 4;
  const FILTER_UNDEFINED = 0;
  const FILTER_POINT = 1;
  const FILTER_BOX = 2;
  const FILTER_TRIANGLE = 3;
  const FILTER_HERMITE = 4;
  const FILTER_HANNING = 5;
  const FILTER_HAMMING = 6;
  const FILTER_BLACKMAN = 7;
  const FILTER_GAUSSIAN = 8;
  const FILTER_QUADRATIC = 9;
  const FILTER_CUBIC = 10;
  const FILTER_CATROM = 11;
  const FILTER_MITCHELL = 12;
  const FILTER_LANCZOS = 22;
  const FILTER_BESSEL = 13;
  const FILTER_SINC = 14;
  const FILTER_KAISER = 16;
  const FILTER_WELSH = 17;
  const FILTER_PARZEN = 18;
  const FILTER_LAGRANGE = 21;
  const FILTER_SENTINEL = 27;
  const FILTER_BOHMAN = 19;
  const FILTER_BARTLETT = 20;
  const FILTER_JINC = 13;
  const FILTER_SINCFAST = 15;
  const FILTER_ROBIDOUX = 26;
  const FILTER_LANCZOSSHARP = 23;
  const FILTER_LANCZOS2 = 24;
  const FILTER_LANCZOS2SHARP = 25;
  const IMGTYPE_UNDEFINED = 0;
  const IMGTYPE_BILEVEL = 1;
  const IMGTYPE_GRAYSCALE = 2;
  const IMGTYPE_GRAYSCALEMATTE = 3;
  const IMGTYPE_PALETTEMATTE = 5;
  const IMGTYPE_TRUECOLOR = 6;
  const IMGTYPE_TRUECOLORMATTE = 7;
  const IMGTYPE_COLORSEPARATION = 8;
  const IMGTYPE_OPTIMIZE = 10;
  const IMGTYPE_PALETTE = 4;
  const RESOLUTION_UNDEFINED = 0;
  const COMPRESSION_UNDEFINED = 0;
  const COMPRESSION_NO = 1;
  const COMPRESSION_BZIP = 2;
  const COMPRESSION_FAX = 6;
  const COMPRESSION_GROUP4 = 7;
  const COMPRESSION_JPEG = 8;
  const COMPRESSION_JPEG2000 = 9;
  const COMPRESSION_LZW = 11;
  const COMPRESSION_RLE = 12;
  const COMPRESSION_ZIP = 13;
  const COMPRESSION_DXT1 = 3;
  const COMPRESSION_DXT3 = 4;
  const COMPRESSION_DXT5 = 5;
  const COMPRESSION_ZIPS = 14;
  const COMPRESSION_PIZ = 15;
  const COMPRESSION_PXR24 = 16;
  const COMPRESSION_B44 = 17;
  const COMPRESSION_B44A = 18;
  const COMPRESSION_LZMA = 19;
  const COMPRESSION_JBIG1 = 20;
  const COMPRESSION_JBIG2 = 21;
  const PAINT_POINT = 1;
  const PAINT_REPLACE = 2;
  const PAINT_FLOODFILL = 3;
  const PAINT_FILLTOBORDER = 4;
  const PAINT_RESET = 5;
  const GRAVITY_NORTHWEST = 1;
  const GRAVITY_NORTH = 2;
  const GRAVITY_NORTHEAST = 3;
  const GRAVITY_WEST = 4;
  const GRAVITY_CENTER = 5;
  const GRAVITY_EAST = 6;
  const GRAVITY_SOUTHWEST = 7;
  const GRAVITY_SOUTH = 8;
  const GRAVITY_SOUTHEAST = 9;
  const STRETCH_NORMAL = 1;
  const STRETCH_ULTRACONDENSED = 2;
  const STRETCH_CONDENSED = 4;
  const STRETCH_SEMICONDENSED = 5;
  const STRETCH_SEMIEXPANDED = 6;
  const STRETCH_EXPANDED = 7;
  const STRETCH_EXTRAEXPANDED = 8;
  const STRETCH_ULTRAEXPANDED = 9;
  const STRETCH_ANY = 10;
  const ALIGN_UNDEFINED = 0;
  const ALIGN_LEFT = 1;
  const ALIGN_CENTER = 2;
  const ALIGN_RIGHT = 3;
  const DECORATION_NO = 1;
  const DECORATION_UNDERLINE = 2;
  const DECORATION_OVERLINE = 3;
  const DECORATION_LINETROUGH = 4;
  const NOISE_UNIFORM = 1;
  const NOISE_GAUSSIAN = 2;
  const NOISE_IMPULSE = 4;
  const NOISE_LAPLACIAN = 5;
  const NOISE_POISSON = 6;
  const NOISE_RANDOM = 7;
  const CHANNEL_UNDEFINED = 0;
  const CHANNEL_RED = 1;
  const CHANNEL_GRAY = 1;
  const CHANNEL_CYAN = 1;
  const CHANNEL_GREEN = 2;
  const CHANNEL_MAGENTA = 2;
  const CHANNEL_BLUE = 4;
  const CHANNEL_YELLOW = 4;
  const CHANNEL_ALPHA = 8;
  const CHANNEL_OPACITY = 8;
  const CHANNEL_MATTE = 8;
  const CHANNEL_BLACK = 32;
  const CHANNEL_INDEX = 32;
  const CHANNEL_ALL = -1;
  const CHANNEL_DEFAULT = -9;
  const CHANNEL_TRUEALPHA = 64;
  const CHANNEL_RGBS = 128;
  const CHANNEL_SYNC = 256;
  const CHANNEL_COMPOSITES = 47;
  const METRIC_UNDEFINED = 0;
  const METRIC_MEANSQUAREERROR = 4;
  const PIXEL_CHAR = 1;
  const PIXEL_DOUBLE = 2;
  const PIXEL_FLOAT = 3;
  const PIXEL_INTEGER = 4;
  const PIXEL_LONG = 5;
  const PIXEL_QUANTUM = 6;
  const PIXEL_SHORT = 7;
  const EVALUATE_UNDEFINED = 0;
  const EVALUATE_ADD = 1;
  const EVALUATE_AND = 2;
  const EVALUATE_DIVIDE = 3;
  const EVALUATE_LEFTSHIFT = 4;
  const EVALUATE_MAX = 5;
  const EVALUATE_MIN = 6;
  const EVALUATE_MULTIPLY = 7;
  const EVALUATE_OR = 8;
  const EVALUATE_RIGHTSHIFT = 9;
  const EVALUATE_SET = 10;
  const EVALUATE_SUBTRACT = 11;
  const EVALUATE_XOR = 12;
  const EVALUATE_POW = 13;
  const EVALUATE_LOG = 14;
  const EVALUATE_THRESHOLD = 15;
  const EVALUATE_THRESHOLDBLACK = 16;
  const EVALUATE_THRESHOLDWHITE = 17;
  const EVALUATE_GAUSSIANNOISE = 18;
  const EVALUATE_IMPULSENOISE = 19;
  const EVALUATE_LAPLACIANNOISE = 20;
  const EVALUATE_POISSONNOISE = 22;
  const EVALUATE_UNIFORMNOISE = 23;
  const EVALUATE_COSINE = 24;
  const EVALUATE_SINE = 25;
  const EVALUATE_ADDMODULUS = 26;
  const EVALUATE_MEAN = 27;
  const EVALUATE_ABS = 28;
  const EVALUATE_EXPONENTIAL = 29;
  const EVALUATE_MEDIAN = 30;
  const COLORSPACE_UNDEFINED = 0;
  const COLORSPACE_RGB = 1;
  const COLORSPACE_GRAY = 2;
  const COLORSPACE_TRANSPARENT = 3;
  const COLORSPACE_OHTA = 4;
  const COLORSPACE_LAB = 5;
  const COLORSPACE_XYZ = 6;
  const COLORSPACE_YCBCR = 7;
  const COLORSPACE_YCC = 8;
  const COLORSPACE_YIQ = 9;
  const COLORSPACE_YPBPR = 10;
  const COLORSPACE_YUV = 11;
  const COLORSPACE_CMYK = 12;
  const COLORSPACE_SRGB = 13;
  const COLORSPACE_HSB = 14;
  const COLORSPACE_HSL = 15;
  const COLORSPACE_HWB = 16;
  const COLORSPACE_REC601LUMA = 17;
  const COLORSPACE_REC709LUMA = 19;
  const COLORSPACE_LOG = 21;
  const COLORSPACE_CMY = 22;
  const VIRTUALPIXELMETHOD_EDGE = 4;
  const VIRTUALPIXELMETHOD_TILE = 7;
  const VIRTUALPIXELMETHOD_MASK = 9;
  const VIRTUALPIXELMETHOD_GRAY = 11;
  const PREVIEW_UNDEFINED = 0;
  const PREVIEW_ROTATE = 1;
  const PREVIEW_SHEAR = 2;
  const PREVIEW_ROLL = 3;
  const PREVIEW_HUE = 4;
  const PREVIEW_SATURATION = 5;
  const PREVIEW_BRIGHTNESS = 6;
  const PREVIEW_GAMMA = 7;
  const PREVIEW_SPIFF = 8;
  const PREVIEW_DULL = 9;
  const PREVIEW_GRAYSCALE = 10;
  const PREVIEW_QUANTIZE = 11;
  const PREVIEW_DESPECKLE = 12;
  const PREVIEW_REDUCENOISE = 13;
  const PREVIEW_ADDNOISE = 14;
  const PREVIEW_SHARPEN = 15;
  const PREVIEW_BLUR = 16;
  const PREVIEW_THRESHOLD = 17;
  const PREVIEW_EDGEDETECT = 18;
  const PREVIEW_SPREAD = 19;
  const PREVIEW_SOLARIZE = 20;
  const PREVIEW_SHADE = 21;
  const PREVIEW_RAISE = 22;
  const PREVIEW_SEGMENT = 23;
  const PREVIEW_SWIRL = 24;
  const PREVIEW_IMPLODE = 25;
  const PREVIEW_WAVE = 26;
  const PREVIEW_OILPAINT = 27;
  const PREVIEW_CHARCOALDRAWING = 28;
  const PREVIEW_JPEG = 29;
  const INTERLACE_UNDEFINED = 0;
  const INTERLACE_NO = 1;
  const INTERLACE_LINE = 2;
  const INTERLACE_PLANE = 3;
  const INTERLACE_PARTITION = 4;
  const INTERLACE_GIF = 5;
  const INTERLACE_JPEG = 6;
  const INTERLACE_PNG = 7;
  const FILLRULE_UNDEFINED = 0;
  const FILLRULE_EVENODD = 1;
  const FILLRULE_NONZERO = 2;
  const PATHUNITS_UNDEFINED = 0;
  const PATHUNITS_USERSPACE = 1;
  const LINECAP_UNDEFINED = 0;
  const LINECAP_BUTT = 1;
  const LINECAP_ROUND = 2;
  const LINECAP_SQUARE = 3;
  const LINEJOIN_UNDEFINED = 0;
  const LINEJOIN_MITER = 1;
  const LINEJOIN_ROUND = 2;
  const LINEJOIN_BEVEL = 3;
  const RESOURCETYPE_UNDEFINED = 0;
  const RESOURCETYPE_AREA = 1;
  const RESOURCETYPE_DISK = 2;
  const RESOURCETYPE_FILE = 3;
  const RESOURCETYPE_MAP = 4;
  const RESOURCETYPE_MEMORY = 5;
  const LAYERMETHOD_UNDEFINED = 0;
  const LAYERMETHOD_COALESCE = 1;
  const LAYERMETHOD_COMPAREANY = 2;
  const LAYERMETHOD_DISPOSE = 5;
  const LAYERMETHOD_OPTIMIZE = 6;
  const LAYERMETHOD_COMPOSITE = 12;
  const LAYERMETHOD_REMOVEDUPS = 10;
  const LAYERMETHOD_REMOVEZERO = 11;
  const LAYERMETHOD_MERGE = 13;
  const LAYERMETHOD_FLATTEN = 14;
  const LAYERMETHOD_MOSAIC = 15;
  const LAYERMETHOD_TRIMBOUNDS = 16;
  const ORIENTATION_UNDEFINED = 0;
  const ORIENTATION_TOPLEFT = 1;
  const ORIENTATION_TOPRIGHT = 2;
  const ORIENTATION_BOTTOMRIGHT = 3;
  const ORIENTATION_BOTTOMLEFT = 4;
  const ORIENTATION_LEFTTOP = 5;
  const ORIENTATION_RIGHTTOP = 6;
  const ORIENTATION_RIGHTBOTTOM = 7;
  const ORIENTATION_LEFTBOTTOM = 8;
  const DISTORTION_UNDEFINED = 0;
  const DISTORTION_AFFINE = 1;
  const DISTORTION_ARC = 9;
  const DISTORTION_BILINEAR = 6;
  const DISTORTION_PERSPECTIVE = 4;
  const DISTORTION_POLYNOMIAL = 8;
  const DISTORTION_POLAR = 10;
  const DISTORTION_DEPOLAR = 11;
  const DISTORTION_BARREL = 14;
  const DISTORTION_SHEPARDS = 16;
  const DISTORTION_SENTINEL = 18;
  const DISTORTION_RESIZE = 17;
  const ALPHACHANNEL_ACTIVATE = 1;
  const ALPHACHANNEL_DEACTIVATE = 4;
  const ALPHACHANNEL_RESET = 7;
  const ALPHACHANNEL_SET = 8;
  const ALPHACHANNEL_UNDEFINED = 0;
  const ALPHACHANNEL_COPY = 3;
  const ALPHACHANNEL_EXTRACT = 5;
  const ALPHACHANNEL_OPAQUE = 6;
  const ALPHACHANNEL_SHAPE = 9;
  const FUNCTION_UNDEFINED = 0;
  const FUNCTION_POLYNOMIAL = 1;
  const FUNCTION_SINUSOID = 2;
  const FUNCTION_ARCSIN = 3;
  const FUNCTION_ARCTAN = 4;
  const INTERPOLATE_UNDEFINED = 0;
  const INTERPOLATE_AVERAGE = 1;
  const INTERPOLATE_BICUBIC = 2;
  const INTERPOLATE_BILINEAR = 3;
  const INTERPOLATE_FILTER = 4;
  const INTERPOLATE_INTEGER = 5;
  const INTERPOLATE_MESH = 6;
  const INTERPOLATE_SPLINE = 8;
  const DITHERMETHOD_UNDEFINED = 0;
  const DITHERMETHOD_NO = 1;
  const DITHERMETHOD_RIEMERSMA = 2;

  // Methods
  public function __get($name);
  public function __isset($name);
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
  public function blurImage(float $radius, float $sigma, int $channel): bool;
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
  ): array;
  public function compareImageLayers(int $method): Imagick;
  public function compareImages(Imagick $compare, int $metric): array;
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
    array $kernel,
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
    array $arguments,
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
  ): array;
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
    array $arguments,
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
  public function getImageBluePrimary(): array;
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
  public function getImageChannelExtrema(int $channel): array;
  public function getImageChannelKurtosis(
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): array;
  public function getImageChannelMean(int $channel): array;
  public function getImageChannelRange(int $channel): array;
  public function getImageChannelStatistics(): array;
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
  public function getImageExtrema(): array;
  public function getImageFilename(): string;
  public function getImageFormat(): string;
  public function getImageGamma(): float;
  public function getImageGeometry(): array;
  public function getImageGravity(): int;
  public function getImageGreenPrimary(): array;
  public function getImageHeight(): int;
  public function getImageHistogram(): array;
  public function getImageIndex(): int;
  public function getImageInterlaceScheme(): int;
  public function getImageInterpolateMethod(): int;
  public function getImageIterations(): int;
  public function getImageLength(): int;
  public function getImageMatte(): bool;
  public function getImageMatteColor(): ImagickPixel;
  public function getImageMimeType(): string;
  public function getImageOrientation(): int;
  public function getImagePage(): array;
  public function getImagePixelColor(int $x, int $y): ImagickPixel;
  public function getImageProfile(string $name): string;
  public function getImageProfiles(
    string $pattern = "*",
    bool $with_values = true,
  ): array;
  public function getImageProperties(
    string $pattern = "*",
    bool $with_values = true,
  ): array;
  public function getImageProperty(string $name): string;
  public function getImageRedPrimary(): array;
  public function getImageRegion(
    int $width,
    int $height,
    int $x,
    int $y,
  ): Imagick;
  public function getImageRenderingIntent(): int;
  public function getImageResolution(): array;
  public function getImagesBlob(): string;
  public function getImageScene(): int;
  public function getImageSignature(): string;
  public function getImageSize(): int;
  public function getImageTicksPerSecond(): int;
  public function getImageTotalInkDensity(): float;
  public function getImageType(): int;
  public function getImageUnits(): int;
  public function getImageVirtualPixelMethod(): int;
  public function getImageWhitePoint(): array;
  public function getImageWidth(): int;
  public function getInterlaceScheme(): int;
  public function getIteratorIndex(): int;
  public function getNumberImages(): int;
  public function getOption(string $key): string;
  public static function getPackageName(): string;
  public function getPage(): array;
  public function getPixelIterator(): ImagickPixelIterator;
  public function getPixelRegionIterator(
    int $x,
    int $y,
    int $columns,
    int $rows,
  ): ImagickPixelIterator;
  public function getPointSize(): float;
  public static function getQuantumDepth(): array;
  public static function getQuantumRange(): array;
  public static function getReleaseDate(): string;
  public static function getResource(int $type): int;
  public static function getResourceLimit(int $type): int;
  public function getSamplingFactors(): array;
  public function getSize(): array;
  public function getSizeOffset(): int;
  public static function getVersion(): array;
  public function haldClutImage(
    Imagick $clut,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function hasNextImage(): bool;
  public function hasPreviousImage(): bool;
  public function identifyImage(bool $appendRawOutput = false): array;
  public function implodeImage(float $radius): bool;
  public function importImagePixels(
    int $x,
    int $y,
    int $width,
    int $height,
    string $map,
    int $storage,
    array $pixels,
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
  ): array;
  public static function queryFonts(string $pattern = "*"): array;
  public static function queryFormats(string $pattern = "*"): array;
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
  public function readImages(array $files): bool;
  public function recolorImage(array $matrix): bool;
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
  public function setSamplingFactors(array $factors): bool;
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
    array $arguments,
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
