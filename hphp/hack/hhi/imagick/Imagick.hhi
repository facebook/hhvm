<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class Imagick implements Countable, Iterator<Imagick>, Traversable<Imagick> {

  // Constants
  const int COMPOSITE_MODULUSSUBTRACT;
  const int COMPOSITE_DARKENINTENSITY;
  const int COMPOSITE_LIGHTENINTENSITY;
  const int IMGTYPE_COLORSEPARATIONMATTE;
  const int IMGTYPE_PALETTEBILEVELMATTE;
  const int RESOLUTION_PIXELSPERINCH;
  const int RESOLUTION_PIXELSPERCENTIMETER;
  const int COMPRESSION_LOSSLESSJPEG;
  const int NOISE_MULTIPLICATIVEGAUSSIAN;
  const int METRIC_MEANABSOLUTEERROR;
  const int METRIC_PEAKABSOLUTEERROR;
  const int METRIC_PEAKSIGNALTONOISERATIO;
  const int METRIC_ROOTMEANSQUAREDERROR;
  const int EVALUATE_MULTIPLICATIVENOISE;
  const int VIRTUALPIXELMETHOD_UNDEFINED;
  const int VIRTUALPIXELMETHOD_BACKGROUND;
  const int VIRTUALPIXELMETHOD_CONSTANT;
  const int VIRTUALPIXELMETHOD_MIRROR;
  const int VIRTUALPIXELMETHOD_TRANSPARENT;
  const int VIRTUALPIXELMETHOD_BLACK;
  const int VIRTUALPIXELMETHOD_WHITE;
  const int VIRTUALPIXELMETHOD_HORIZONTALTILE;
  const int VIRTUALPIXELMETHOD_VERTICALTILE;
  const int VIRTUALPIXELMETHOD_HORIZONTALTILEEDGE;
  const int VIRTUALPIXELMETHOD_VERTICALTILEEDGE;
  const int VIRTUALPIXELMETHOD_CHECKERTILE;
  const int RENDERINGINTENT_UNDEFINED;
  const int RENDERINGINTENT_SATURATION;
  const int RENDERINGINTENT_PERCEPTUAL;
  const int RENDERINGINTENT_ABSOLUTE;
  const int RENDERINGINTENT_RELATIVE;
  const int PATHUNITS_USERSPACEONUSE;
  const int PATHUNITS_OBJECTBOUNDINGBOX;
  const int LAYERMETHOD_COMPARECLEAR;
  const int LAYERMETHOD_COMPAREOVERLAY;
  const int LAYERMETHOD_OPTIMIZEPLUS;
  const int LAYERMETHOD_OPTIMIZETRANS;
  const int LAYERMETHOD_OPTIMIZEIMAGE;
  const int DISTORTION_AFFINEPROJECTION;
  const int DISTORTION_PERSPECTIVEPROJECTION;
  const int DISTORTION_SCALEROTATETRANSLATE;
  const int DISTORTION_BARRELINVERSE;
  const int DISTORTION_BILINEARFORWARD;
  const int DISTORTION_BILINEARREVERSE;
  const int DISTORTION_CYLINDER2PLANE;
  const int DISTORTION_PLANE2CYLINDER;
  const int ALPHACHANNEL_TRANSPARENT;
  const int SPARSECOLORMETHOD_UNDEFINED;
  const int SPARSECOLORMETHOD_BARYCENTRIC;
  const int SPARSECOLORMETHOD_BILINEAR;
  const int SPARSECOLORMETHOD_POLYNOMIAL;
  const int SPARSECOLORMETHOD_SPEPARDS;
  const int SPARSECOLORMETHOD_VORONOI;
  const int INTERPOLATE_NEARESTNEIGHBOR;
  const int DITHERMETHOD_FLOYDSTEINBERG;
  const int COLOR_BLACK;
  const int COLOR_BLUE;
  const int COLOR_CYAN;
  const int COLOR_GREEN;
  const int COLOR_RED;
  const int COLOR_YELLOW;
  const int COLOR_MAGENTA;
  const int COLOR_OPACITY;
  const int COLOR_ALPHA;
  const int COLOR_FUZZ;
  const int DISPOSE_UNRECOGNIZED;
  const int DISPOSE_UNDEFINED;
  const int DISPOSE_NONE;
  const int DISPOSE_BACKGROUND;
  const int DISPOSE_PREVIOUS;
  const int COMPOSITE_DEFAULT;
  const int COMPOSITE_UNDEFINED;
  const int COMPOSITE_NO;
  const int COMPOSITE_ADD;
  const int COMPOSITE_ATOP;
  const int COMPOSITE_BLEND;
  const int COMPOSITE_BUMPMAP;
  const int COMPOSITE_CLEAR;
  const int COMPOSITE_COLORBURN;
  const int COMPOSITE_COLORDODGE;
  const int COMPOSITE_COLORIZE;
  const int COMPOSITE_COPYBLACK;
  const int COMPOSITE_COPYBLUE;
  const int COMPOSITE_COPY;
  const int COMPOSITE_COPYCYAN;
  const int COMPOSITE_COPYGREEN;
  const int COMPOSITE_COPYMAGENTA;
  const int COMPOSITE_COPYOPACITY;
  const int COMPOSITE_COPYRED;
  const int COMPOSITE_COPYYELLOW;
  const int COMPOSITE_DARKEN;
  const int COMPOSITE_DSTATOP;
  const int COMPOSITE_DST;
  const int COMPOSITE_DSTIN;
  const int COMPOSITE_DSTOUT;
  const int COMPOSITE_DSTOVER;
  const int COMPOSITE_DIFFERENCE;
  const int COMPOSITE_DISPLACE;
  const int COMPOSITE_DISSOLVE;
  const int COMPOSITE_EXCLUSION;
  const int COMPOSITE_HARDLIGHT;
  const int COMPOSITE_HUE;
  const int COMPOSITE_IN;
  const int COMPOSITE_LIGHTEN;
  const int COMPOSITE_LUMINIZE;
  const int COMPOSITE_MINUS;
  const int COMPOSITE_MODULATE;
  const int COMPOSITE_MULTIPLY;
  const int COMPOSITE_OUT;
  const int COMPOSITE_OVER;
  const int COMPOSITE_OVERLAY;
  const int COMPOSITE_PLUS;
  const int COMPOSITE_REPLACE;
  const int COMPOSITE_SATURATE;
  const int COMPOSITE_SCREEN;
  const int COMPOSITE_SOFTLIGHT;
  const int COMPOSITE_SRCATOP;
  const int COMPOSITE_SRC;
  const int COMPOSITE_SRCIN;
  const int COMPOSITE_SRCOUT;
  const int COMPOSITE_SRCOVER;
  const int COMPOSITE_SUBTRACT;
  const int COMPOSITE_THRESHOLD;
  const int COMPOSITE_XOR;
  const int COMPOSITE_CHANGEMASK;
  const int COMPOSITE_LINEARLIGHT;
  const int COMPOSITE_DIVIDE;
  const int COMPOSITE_DISTORT;
  const int COMPOSITE_BLUR;
  const int COMPOSITE_PEGTOPLIGHT;
  const int COMPOSITE_VIVIDLIGHT;
  const int COMPOSITE_PINLIGHT;
  const int COMPOSITE_LINEARDODGE;
  const int COMPOSITE_LINEARBURN;
  const int COMPOSITE_MATHEMATICS;
  const int COMPOSITE_MODULUSADD;
  const int COMPOSITE_MINUSDST;
  const int COMPOSITE_DIVIDEDST;
  const int COMPOSITE_DIVIDESRC;
  const int COMPOSITE_MINUSSRC;
  const int MONTAGEMODE_FRAME;
  const int MONTAGEMODE_UNFRAME;
  const int MONTAGEMODE_CONCATENATE;
  const int STYLE_NORMAL;
  const int STYLE_ITALIC;
  const int STYLE_OBLIQUE;
  const int STYLE_ANY;
  const int FILTER_UNDEFINED;
  const int FILTER_POINT;
  const int FILTER_BOX;
  const int FILTER_TRIANGLE;
  const int FILTER_HERMITE;
  const int FILTER_HANNING;
  const int FILTER_HAMMING;
  const int FILTER_BLACKMAN;
  const int FILTER_GAUSSIAN;
  const int FILTER_QUADRATIC;
  const int FILTER_CUBIC;
  const int FILTER_CATROM;
  const int FILTER_MITCHELL;
  const int FILTER_LANCZOS;
  const int FILTER_BESSEL;
  const int FILTER_SINC;
  const int FILTER_KAISER;
  const int FILTER_WELSH;
  const int FILTER_PARZEN;
  const int FILTER_LAGRANGE;
  const int FILTER_SENTINEL;
  const int FILTER_BOHMAN;
  const int FILTER_BARTLETT;
  const int FILTER_JINC;
  const int FILTER_SINCFAST;
  const int FILTER_ROBIDOUX;
  const int FILTER_LANCZOSSHARP;
  const int FILTER_LANCZOS2;
  const int FILTER_LANCZOS2SHARP;
  const int IMGTYPE_UNDEFINED;
  const int IMGTYPE_BILEVEL;
  const int IMGTYPE_GRAYSCALE;
  const int IMGTYPE_GRAYSCALEMATTE;
  const int IMGTYPE_PALETTEMATTE;
  const int IMGTYPE_TRUECOLOR;
  const int IMGTYPE_TRUECOLORMATTE;
  const int IMGTYPE_COLORSEPARATION;
  const int IMGTYPE_OPTIMIZE;
  const int IMGTYPE_PALETTE;
  const int RESOLUTION_UNDEFINED;
  const int COMPRESSION_UNDEFINED;
  const int COMPRESSION_NO;
  const int COMPRESSION_BZIP;
  const int COMPRESSION_FAX;
  const int COMPRESSION_GROUP4;
  const int COMPRESSION_JPEG;
  const int COMPRESSION_JPEG2000;
  const int COMPRESSION_LZW;
  const int COMPRESSION_RLE;
  const int COMPRESSION_ZIP;
  const int COMPRESSION_DXT1;
  const int COMPRESSION_DXT3;
  const int COMPRESSION_DXT5;
  const int COMPRESSION_ZIPS;
  const int COMPRESSION_PIZ;
  const int COMPRESSION_PXR24;
  const int COMPRESSION_B44;
  const int COMPRESSION_B44A;
  const int COMPRESSION_LZMA;
  const int COMPRESSION_JBIG1;
  const int COMPRESSION_JBIG2;
  const int PAINT_POINT;
  const int PAINT_REPLACE;
  const int PAINT_FLOODFILL;
  const int PAINT_FILLTOBORDER;
  const int PAINT_RESET;
  const int GRAVITY_NORTHWEST;
  const int GRAVITY_NORTH;
  const int GRAVITY_NORTHEAST;
  const int GRAVITY_WEST;
  const int GRAVITY_CENTER;
  const int GRAVITY_EAST;
  const int GRAVITY_SOUTHWEST;
  const int GRAVITY_SOUTH;
  const int GRAVITY_SOUTHEAST;
  const int STRETCH_NORMAL;
  const int STRETCH_ULTRACONDENSED;
  const int STRETCH_CONDENSED;
  const int STRETCH_SEMICONDENSED;
  const int STRETCH_SEMIEXPANDED;
  const int STRETCH_EXPANDED;
  const int STRETCH_EXTRAEXPANDED;
  const int STRETCH_ULTRAEXPANDED;
  const int STRETCH_ANY;
  const int ALIGN_UNDEFINED;
  const int ALIGN_LEFT;
  const int ALIGN_CENTER;
  const int ALIGN_RIGHT;
  const int DECORATION_NO;
  const int DECORATION_UNDERLINE;
  const int DECORATION_OVERLINE;
  const int DECORATION_LINETROUGH;
  const int NOISE_UNIFORM;
  const int NOISE_GAUSSIAN;
  const int NOISE_IMPULSE;
  const int NOISE_LAPLACIAN;
  const int NOISE_POISSON;
  const int NOISE_RANDOM;
  const int CHANNEL_UNDEFINED;
  const int CHANNEL_RED;
  const int CHANNEL_GRAY;
  const int CHANNEL_CYAN;
  const int CHANNEL_GREEN;
  const int CHANNEL_MAGENTA;
  const int CHANNEL_BLUE;
  const int CHANNEL_YELLOW;
  const int CHANNEL_ALPHA;
  const int CHANNEL_OPACITY;
  const int CHANNEL_MATTE;
  const int CHANNEL_BLACK;
  const int CHANNEL_INDEX;
  const int CHANNEL_ALL;
  const int CHANNEL_DEFAULT;
  const int CHANNEL_TRUEALPHA;
  const int CHANNEL_RGBS;
  const int CHANNEL_SYNC;
  const int CHANNEL_COMPOSITES;
  const int METRIC_UNDEFINED;
  const int METRIC_MEANSQUAREERROR;
  const int PIXEL_CHAR;
  const int PIXEL_DOUBLE;
  const int PIXEL_FLOAT;
  const int PIXEL_INTEGER;
  const int PIXEL_LONG;
  const int PIXEL_QUANTUM;
  const int PIXEL_SHORT;
  const int EVALUATE_UNDEFINED;
  const int EVALUATE_ADD;
  const int EVALUATE_AND;
  const int EVALUATE_DIVIDE;
  const int EVALUATE_LEFTSHIFT;
  const int EVALUATE_MAX;
  const int EVALUATE_MIN;
  const int EVALUATE_MULTIPLY;
  const int EVALUATE_OR;
  const int EVALUATE_RIGHTSHIFT;
  const int EVALUATE_SET;
  const int EVALUATE_SUBTRACT;
  const int EVALUATE_XOR;
  const int EVALUATE_POW;
  const int EVALUATE_LOG;
  const int EVALUATE_THRESHOLD;
  const int EVALUATE_THRESHOLDBLACK;
  const int EVALUATE_THRESHOLDWHITE;
  const int EVALUATE_GAUSSIANNOISE;
  const int EVALUATE_IMPULSENOISE;
  const int EVALUATE_LAPLACIANNOISE;
  const int EVALUATE_POISSONNOISE;
  const int EVALUATE_UNIFORMNOISE;
  const int EVALUATE_COSINE;
  const int EVALUATE_SINE;
  const int EVALUATE_ADDMODULUS;
  const int EVALUATE_MEAN;
  const int EVALUATE_ABS;
  const int EVALUATE_EXPONENTIAL;
  const int EVALUATE_MEDIAN;
  const int COLORSPACE_UNDEFINED;
  const int COLORSPACE_RGB;
  const int COLORSPACE_GRAY;
  const int COLORSPACE_TRANSPARENT;
  const int COLORSPACE_OHTA;
  const int COLORSPACE_LAB;
  const int COLORSPACE_XYZ;
  const int COLORSPACE_YCBCR;
  const int COLORSPACE_YCC;
  const int COLORSPACE_YIQ;
  const int COLORSPACE_YPBPR;
  const int COLORSPACE_YUV;
  const int COLORSPACE_CMYK;
  const int COLORSPACE_SRGB;
  const int COLORSPACE_HSB;
  const int COLORSPACE_HSL;
  const int COLORSPACE_HWB;
  const int COLORSPACE_REC601LUMA;
  const int COLORSPACE_REC709LUMA;
  const int COLORSPACE_LOG;
  const int COLORSPACE_CMY;
  const int VIRTUALPIXELMETHOD_EDGE;
  const int VIRTUALPIXELMETHOD_TILE;
  const int VIRTUALPIXELMETHOD_MASK;
  const int VIRTUALPIXELMETHOD_GRAY;
  const int PREVIEW_UNDEFINED;
  const int PREVIEW_ROTATE;
  const int PREVIEW_SHEAR;
  const int PREVIEW_ROLL;
  const int PREVIEW_HUE;
  const int PREVIEW_SATURATION;
  const int PREVIEW_BRIGHTNESS;
  const int PREVIEW_GAMMA;
  const int PREVIEW_SPIFF;
  const int PREVIEW_DULL;
  const int PREVIEW_GRAYSCALE;
  const int PREVIEW_QUANTIZE;
  const int PREVIEW_DESPECKLE;
  const int PREVIEW_REDUCENOISE;
  const int PREVIEW_ADDNOISE;
  const int PREVIEW_SHARPEN;
  const int PREVIEW_BLUR;
  const int PREVIEW_THRESHOLD;
  const int PREVIEW_EDGEDETECT;
  const int PREVIEW_SPREAD;
  const int PREVIEW_SOLARIZE;
  const int PREVIEW_SHADE;
  const int PREVIEW_RAISE;
  const int PREVIEW_SEGMENT;
  const int PREVIEW_SWIRL;
  const int PREVIEW_IMPLODE;
  const int PREVIEW_WAVE;
  const int PREVIEW_OILPAINT;
  const int PREVIEW_CHARCOALDRAWING;
  const int PREVIEW_JPEG;
  const int INTERLACE_UNDEFINED;
  const int INTERLACE_NO;
  const int INTERLACE_LINE;
  const int INTERLACE_PLANE;
  const int INTERLACE_PARTITION;
  const int INTERLACE_GIF;
  const int INTERLACE_JPEG;
  const int INTERLACE_PNG;
  const int FILLRULE_UNDEFINED;
  const int FILLRULE_EVENODD;
  const int FILLRULE_NONZERO;
  const int PATHUNITS_UNDEFINED;
  const int PATHUNITS_USERSPACE;
  const int LINECAP_UNDEFINED;
  const int LINECAP_BUTT;
  const int LINECAP_ROUND;
  const int LINECAP_SQUARE;
  const int LINEJOIN_UNDEFINED;
  const int LINEJOIN_MITER;
  const int LINEJOIN_ROUND;
  const int LINEJOIN_BEVEL;
  const int RESOURCETYPE_UNDEFINED;
  const int RESOURCETYPE_AREA;
  const int RESOURCETYPE_DISK;
  const int RESOURCETYPE_FILE;
  const int RESOURCETYPE_MAP;
  const int RESOURCETYPE_MEMORY;
  const int LAYERMETHOD_UNDEFINED;
  const int LAYERMETHOD_COALESCE;
  const int LAYERMETHOD_COMPAREANY;
  const int LAYERMETHOD_DISPOSE;
  const int LAYERMETHOD_OPTIMIZE;
  const int LAYERMETHOD_COMPOSITE;
  const int LAYERMETHOD_REMOVEDUPS;
  const int LAYERMETHOD_REMOVEZERO;
  const int LAYERMETHOD_MERGE;
  const int LAYERMETHOD_FLATTEN;
  const int LAYERMETHOD_MOSAIC;
  const int LAYERMETHOD_TRIMBOUNDS;
  const int ORIENTATION_UNDEFINED;
  const int ORIENTATION_TOPLEFT;
  const int ORIENTATION_TOPRIGHT;
  const int ORIENTATION_BOTTOMRIGHT;
  const int ORIENTATION_BOTTOMLEFT;
  const int ORIENTATION_LEFTTOP;
  const int ORIENTATION_RIGHTTOP;
  const int ORIENTATION_RIGHTBOTTOM;
  const int ORIENTATION_LEFTBOTTOM;
  const int DISTORTION_UNDEFINED;
  const int DISTORTION_AFFINE;
  const int DISTORTION_ARC;
  const int DISTORTION_BILINEAR;
  const int DISTORTION_PERSPECTIVE;
  const int DISTORTION_POLYNOMIAL;
  const int DISTORTION_POLAR;
  const int DISTORTION_DEPOLAR;
  const int DISTORTION_BARREL;
  const int DISTORTION_SHEPARDS;
  const int DISTORTION_SENTINEL;
  const int DISTORTION_RESIZE;
  const int ALPHACHANNEL_ACTIVATE;
  const int ALPHACHANNEL_DEACTIVATE;
  const int ALPHACHANNEL_RESET;
  const int ALPHACHANNEL_SET;
  const int ALPHACHANNEL_UNDEFINED;
  const int ALPHACHANNEL_COPY;
  const int ALPHACHANNEL_EXTRACT;
  const int ALPHACHANNEL_OPAQUE;
  const int ALPHACHANNEL_SHAPE;
  const int FUNCTION_UNDEFINED;
  const int FUNCTION_POLYNOMIAL;
  const int FUNCTION_SINUSOID;
  const int FUNCTION_ARCSIN;
  const int FUNCTION_ARCTAN;
  const int INTERPOLATE_UNDEFINED;
  const int INTERPOLATE_AVERAGE;
  const int INTERPOLATE_BICUBIC;
  const int INTERPOLATE_BILINEAR;
  const int INTERPOLATE_FILTER;
  const int INTERPOLATE_INTEGER;
  const int INTERPOLATE_MESH;
  const int INTERPOLATE_SPLINE;
  const int DITHERMETHOD_UNDEFINED;
  const int DITHERMETHOD_NO;
  const int DITHERMETHOD_RIEMERSMA;

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
  public function blackThresholdImage(
    HH\FIXME\MISSING_PARAM_TYPE $threshold,
  ): bool;
  public function blurImage(
    float $radius,
    float $sigma,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function borderImage(
    HH\FIXME\MISSING_PARAM_TYPE $bordercolor,
    int $width,
    int $height,
  ): bool;
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
    HH\FIXME\MISSING_PARAM_TYPE $fill,
    float $fuzz,
    HH\FIXME\MISSING_PARAM_TYPE $bordercolor,
    int $x,
    int $y,
  ): bool;
  public function colorizeImage(
    HH\FIXME\MISSING_PARAM_TYPE $colorize,
    HH\FIXME\MISSING_PARAM_TYPE $opacity,
  ): bool;
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
  public function __construct(HH\FIXME\MISSING_PARAM_TYPE $files = null);
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
    HH\FIXME\MISSING_PARAM_TYPE $fill,
    float $fuzz,
    HH\FIXME\MISSING_PARAM_TYPE $target,
    int $x,
    int $y,
    bool $invert,
    int $channel = \Imagick::CHANNEL_DEFAULT,
  ): bool;
  public function flopImage(): bool;
  public function frameImage(
    HH\FIXME\MISSING_PARAM_TYPE $matte_color,
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
    HH\FIXME\MISSING_PARAM_TYPE $bordercolor,
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
    HH\FIXME\MISSING_PARAM_TYPE $background,
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
    HH\FIXME\MISSING_PARAM_TYPE $target,
    HH\FIXME\MISSING_PARAM_TYPE $fill,
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
    HH\FIXME\MISSING_PARAM_TYPE $fill,
    float $fuzz,
    HH\FIXME\MISSING_PARAM_TYPE $bordercolor,
    int $x,
    int $y,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function paintOpaqueImage(
    HH\FIXME\MISSING_PARAM_TYPE $target,
    HH\FIXME\MISSING_PARAM_TYPE $fill,
    float $fuzz,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function paintTransparentImage(
    HH\FIXME\MISSING_PARAM_TYPE $target,
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
    HH\FIXME\MISSING_PARAM_TYPE $multiline = null,
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
  public function rotateImage(
    HH\FIXME\MISSING_PARAM_TYPE $background,
    float $degrees,
  ): bool;
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
  public function scaleImage(int $cols, int $rows, bool $bestfit = false): bool;
  public function segmentImage(
    int $COLORSPACE,
    float $cluster_threshold,
    float $smooth_threshold,
    bool $verbose = false,
  ): bool;
  public function separateImageChannel(int $channel): bool;
  public function sepiaToneImage(float $threshold): bool;
  public function setBackgroundColor(
    HH\FIXME\MISSING_PARAM_TYPE $background,
  ): bool;
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
  public function setImageBackgroundColor(
    HH\FIXME\MISSING_PARAM_TYPE $background,
  ): bool;
  public function setImageBias(float $bias): bool;
  public function setImageBluePrimary(float $x, float $y): bool;
  public function setImageBorderColor(
    HH\FIXME\MISSING_PARAM_TYPE $border,
  ): bool;
  public function setImageChannelDepth(int $channel, int $depth): bool;
  public function setImageClipMask(Imagick $clip_mask): bool;
  public function setImageColormapColor(
    int $index,
    HH\FIXME\MISSING_PARAM_TYPE $color,
  ): bool;
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
  public function setImageMatteColor(HH\FIXME\MISSING_PARAM_TYPE $matte): bool;
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
  public function setResolution(float $x_resolution, float $y_resolution): bool;
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
    HH\FIXME\MISSING_PARAM_TYPE $background,
    float $x_shear,
    float $y_shear,
  ): bool;
  public function sigmoidalContrastImage(
    bool $sharpen,
    float $alpha,
    float $beta,
    int $channel = \Imagick::CHANNEL_ALL,
  ): bool;
  public function sketchImage(float $radius, float $sigma, float $angle): bool;
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
  public function tintImage(
    HH\FIXME\MISSING_PARAM_TYPE $tint,
    HH\FIXME\MISSING_PARAM_TYPE $opacity,
  ): bool;
  public function transformImage(string $crop, string $geometry): Imagick;
  public function transparentPaintImage(
    HH\FIXME\MISSING_PARAM_TYPE $target,
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
  public function whiteThresholdImage(
    HH\FIXME\MISSING_PARAM_TYPE $threshold,
  ): bool;
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
