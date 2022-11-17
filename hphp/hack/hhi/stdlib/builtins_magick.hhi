<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MW_AbsoluteIntent;
const int MW_AddCompositeOp;
const int MW_AddEvaluateOperator;
const int MW_AddNoisePreview;
const int MW_AllChannels;
const int MW_AlphaChannel;
const int MW_AndEvaluateOperator;
const int MW_AnyStretch;
const int MW_AnyStyle;
const int MW_AreaResource;
const int MW_AtopCompositeOp;
const int MW_BZipCompression;
const int MW_BackgroundDispose;
const int MW_BesselFilter;
const int MW_BevelJoin;
const int MW_BilevelType;
const int MW_BlackChannel;
const int MW_BlackmanFilter;
const int MW_BlendCompositeOp;
const int MW_BlobError;
const int MW_BlobFatalError;
const int MW_BlobWarning;
const int MW_BlueChannel;
const int MW_BlurPreview;
const int MW_BoxFilter;
const int MW_BrightnessPreview;
const int MW_BumpmapCompositeOp;
const int MW_ButtCap;
const int MW_CMYKColorspace;
const int MW_CacheError;
const int MW_CacheFatalError;
const int MW_CacheWarning;
const int MW_CatromFilter;
const int MW_CenterAlign;
const int MW_CenterGravity;
const int MW_CharPixel;
const int MW_CharcoalDrawingPreview;
const int MW_ClearCompositeOp;
const int MW_CoderError;
const int MW_CoderFatalError;
const int MW_CoderWarning;
const int MW_ColorBurnCompositeOp;
const int MW_ColorDodgeCompositeOp;
const int MW_ColorSeparationMatteType;
const int MW_ColorSeparationType;
const int MW_ColorizeCompositeOp;
const int MW_ConcatenateMode;
const int MW_CondensedStretch;
const int MW_ConfigureError;
const int MW_ConfigureFatalError;
const int MW_ConfigureWarning;
const int MW_ConstantVirtualPixelMethod;
const int MW_CopyBlackCompositeOp;
const int MW_CopyBlueCompositeOp;
const int MW_CopyCompositeOp;
const int MW_CopyCyanCompositeOp;
const int MW_CopyGreenCompositeOp;
const int MW_CopyMagentaCompositeOp;
const int MW_CopyOpacityCompositeOp;
const int MW_CopyRedCompositeOp;
const int MW_CopyYellowCompositeOp;
const int MW_CorruptImageError;
const int MW_CorruptImageFatalError;
const int MW_CorruptImageWarning;
const int MW_CubicFilter;
const int MW_CyanChannel;
const int MW_DarkenCompositeOp;
const int MW_DelegateError;
const int MW_DelegateFatalError;
const int MW_DelegateWarning;
const int MW_DespecklePreview;
const int MW_DifferenceCompositeOp;
const int MW_DiskResource;
const int MW_DisplaceCompositeOp;
const int MW_DissolveCompositeOp;
const int MW_DivideEvaluateOperator;
const int MW_DoublePixel;
const int MW_DrawError;
const int MW_DrawFatalError;
const int MW_DrawWarning;
const int MW_DstAtopCompositeOp;
const int MW_DstCompositeOp;
const int MW_DstInCompositeOp;
const int MW_DstOutCompositeOp;
const int MW_DstOverCompositeOp;
const int MW_DullPreview;
const int MW_EastGravity;
const int MW_EdgeDetectPreview;
const int MW_EdgeVirtualPixelMethod;
const int MW_ErrorException;
const int MW_EvenOddRule;
const int MW_ExclusionCompositeOp;
const int MW_ExpandedStretch;
const int MW_ExtraCondensedStretch;
const int MW_ExtraExpandedStretch;
const int MW_FatalErrorException;
const int MW_FaxCompression;
const int MW_FileOpenError;
const int MW_FileOpenFatalError;
const int MW_FileOpenWarning;
const int MW_FileResource;
const int MW_FillToBorderMethod;
const int MW_FloatPixel;
const int MW_FloodfillMethod;
const int MW_ForgetGravity;
const int MW_FrameMode;
const int MW_GRAYColorspace;
const int MW_GammaPreview;
const int MW_GaussianFilter;
const int MW_GaussianNoise;
const int MW_GrayscaleMatteType;
const int MW_GrayscalePreview;
const int MW_GrayscaleType;
const int MW_GreenChannel;
const int MW_Group4Compression;
const int MW_HSBColorspace;
const int MW_HSLColorspace;
const int MW_HWBColorspace;
const int MW_HammingFilter;
const int MW_HanningFilter;
const int MW_HardLightCompositeOp;
const int MW_HermiteFilter;
const int MW_HueCompositeOp;
const int MW_HuePreview;
const int MW_ImageError;
const int MW_ImageFatalError;
const int MW_ImageWarning;
const int MW_ImplodePreview;
const int MW_ImpulseNoise;
const int MW_InCompositeOp;
const int MW_IndexChannel;
const int MW_IntegerPixel;
const int MW_ItalicStyle;
const int MW_JPEGCompression;
const int MW_JPEGPreview;
const int MW_LABColorspace;
const int MW_LZWCompression;
const int MW_LanczosFilter;
const int MW_LaplacianNoise;
const int MW_LeftAlign;
const int MW_LeftShiftEvaluateOperator;
const int MW_LightenCompositeOp;
const int MW_LineInterlace;
const int MW_LineThroughDecoration;
const int MW_LongPixel;
const int MW_LosslessJPEGCompression;
const int MW_LuminizeCompositeOp;
const int MW_MagentaChannel;
const int MW_MapResource;
const int MW_MaxEvaluateOperator;
const int MW_MaxRGB;
const int MW_MeanAbsoluteErrorMetric;
const int MW_MeanSquaredErrorMetric;
const int MW_MemoryResource;
const int MW_MinEvaluateOperator;
const int MW_MinusCompositeOp;
const int MW_MirrorVirtualPixelMethod;
const int MW_MissingDelegateError;
const int MW_MissingDelegateFatalError;
const int MW_MissingDelegateWarning;
const int MW_MitchellFilter;
const int MW_MiterJoin;
const int MW_ModulateCompositeOp;
const int MW_ModuleError;
const int MW_ModuleFatalError;
const int MW_ModuleWarning;
const int MW_MonitorError;
const int MW_MonitorFatalError;
const int MW_MonitorWarning;
const int MW_MultiplicativeGaussianNoise;
const int MW_MultiplyCompositeOp;
const int MW_MultiplyEvaluateOperator;
const int MW_NoCompositeOp;
const int MW_NoCompression;
const int MW_NoDecoration;
const int MW_NoInterlace;
const int MW_NonZeroRule;
const int MW_NoneDispose;
const int MW_NormalStretch;
const int MW_NormalStyle;
const int MW_NorthEastGravity;
const int MW_NorthGravity;
const int MW_NorthWestGravity;
const int MW_OHTAColorspace;
const int MW_ObjectBoundingBox;
const int MW_ObliqueStyle;
const int MW_OilPaintPreview;
const int MW_OpacityChannel;
const int MW_OpaqueOpacity;
const int MW_OptimizeType;
const int MW_OptionError;
const int MW_OptionFatalError;
const int MW_OptionWarning;
const int MW_OrEvaluateOperator;
const int MW_OutCompositeOp;
const int MW_OverCompositeOp;
const int MW_OverlayCompositeOp;
const int MW_OverlineDecoration;
const int MW_PaletteMatteType;
const int MW_PaletteType;
const int MW_PartitionInterlace;
const int MW_PeakAbsoluteErrorMetric;
const int MW_PeakSignalToNoiseRatioMetric;
const int MW_PerceptualIntent;
const int MW_PixelsPerCentimeterResolution;
const int MW_PixelsPerInchResolution;
const int MW_PlaneInterlace;
const int MW_PlusCompositeOp;
const int MW_PointFilter;
const int MW_PointMethod;
const int MW_PoissonNoise;
const int MW_PreviousDispose;
const int MW_QuadraticFilter;
const int MW_QuantizePreview;
const int MW_QuantumRange;
const int MW_RGBColorspace;
const int MW_RLECompression;
const int MW_RaisePreview;
const int MW_RedChannel;
const int MW_ReduceNoisePreview;
const int MW_RegistryError;
const int MW_RegistryFatalError;
const int MW_RegistryWarning;
const int MW_RelativeIntent;
const int MW_ReplaceCompositeOp;
const int MW_ReplaceMethod;
const int MW_ResetMethod;
const int MW_ResourceLimitError;
const int MW_ResourceLimitFatalError;
const int MW_ResourceLimitWarning;
const int MW_RightAlign;
const int MW_RightShiftEvaluateOperator;
const int MW_RollPreview;
const int MW_RootMeanSquaredErrorMetric;
const int MW_RotatePreview;
const int MW_RoundCap;
const int MW_RoundJoin;
const int MW_SaturateCompositeOp;
const int MW_SaturationIntent;
const int MW_SaturationPreview;
const int MW_ScreenCompositeOp;
const int MW_SegmentPreview;
const int MW_SemiCondensedStretch;
const int MW_SemiExpandedStretch;
const int MW_SetEvaluateOperator;
const int MW_ShadePreview;
const int MW_SharpenPreview;
const int MW_ShearPreview;
const int MW_ShortPixel;
const int MW_SincFilter;
const int MW_SoftLightCompositeOp;
const int MW_SolarizePreview;
const int MW_SouthEastGravity;
const int MW_SouthGravity;
const int MW_SouthWestGravity;
const int MW_SpiffPreview;
const int MW_SpreadPreview;
const int MW_SquareCap;
const int MW_SrcAtopCompositeOp;
const int MW_SrcCompositeOp;
const int MW_SrcInCompositeOp;
const int MW_SrcOutCompositeOp;
const int MW_SrcOverCompositeOp;
const int MW_StaticGravity;
const int MW_StreamError;
const int MW_StreamFatalError;
const int MW_StreamWarning;
const int MW_SubtractCompositeOp;
const int MW_SubtractEvaluateOperator;
const int MW_SwirlPreview;
const int MW_ThresholdCompositeOp;
const int MW_ThresholdPreview;
const int MW_TileVirtualPixelMethod;
const int MW_TransparentColorspace;
const int MW_TransparentOpacity;
const int MW_TriangleFilter;
const int MW_TrueColorMatteType;
const int MW_TrueColorType;
const int MW_TypeError;
const int MW_TypeFatalError;
const int MW_TypeWarning;
const int MW_UltraCondensedStretch;
const int MW_UltraExpandedStretch;
const int MW_UndefinedAlign;
const int MW_UndefinedCap;
const int MW_UndefinedChannel;
const int MW_UndefinedColorspace;
const int MW_UndefinedCompositeOp;
const int MW_UndefinedCompression;
const int MW_UndefinedDecoration;
const int MW_UndefinedDispose;
const int MW_UndefinedEvaluateOperator;
const int MW_UndefinedException;
const int MW_UndefinedFilter;
const int MW_UndefinedGravity;
const int MW_UndefinedIntent;
const int MW_UndefinedInterlace;
const int MW_UndefinedJoin;
const int MW_UndefinedMethod;
const int MW_UndefinedMetric;
const int MW_UndefinedMode;
const int MW_UndefinedNoise;
const int MW_UndefinedPathUnits;
const int MW_UndefinedPixel;
const int MW_UndefinedPreview;
const int MW_UndefinedResolution;
const int MW_UndefinedResource;
const int MW_UndefinedRule;
const int MW_UndefinedStretch;
const int MW_UndefinedStyle;
const int MW_UndefinedType;
const int MW_UndefinedVirtualPixelMethod;
const int MW_UnderlineDecoration;
const int MW_UnframeMode;
const int MW_UniformNoise;
const int MW_UnrecognizedDispose;
const int MW_UserSpace;
const int MW_UserSpaceOnUse;
const int MW_WandError;
const int MW_WandFatalError;
const int MW_WandWarning;
const int MW_WarningException;
const int MW_WavePreview;
const int MW_WestGravity;
const int MW_XYZColorspace;
const int MW_XorCompositeOp;
const int MW_XorEvaluateOperator;
const int MW_YCCColorspace;
const int MW_YCbCrColorspace;
const int MW_YIQColorspace;
const int MW_YPbPrColorspace;
const int MW_YUVColorspace;
const int MW_YellowChannel;
const int MW_ZipCompression;
const int MW_sRGBColorspace;

<<__PHPStdLib>>
function magickgetcopyright(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgethomeurl(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetpackagename(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetquantumdepth(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetreleasedate(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetresourcelimit($resource_type): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversion(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversionnumber(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversionstring(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryconfigureoption($option): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryconfigureoptions($pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryfonts($pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryformats($pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetresourcelimit(
  $resource_type,
  $limit,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newdrawingwand(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newmagickwand(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixeliterator($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelregioniterator(
  $mgck_wnd,
  $x,
  $y,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwand(
  $imagemagick_col_str = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwandarray($num_pxl_wnds): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwands($num_pxl_wnds): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroydrawingwand($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroymagickwand($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixeliterator($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwand($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwandarray($pxl_wnd_array): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwands($pxl_wnd_array): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function isdrawingwand($var): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ismagickwand($var): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ispixeliterator($var): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ispixelwand($var): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function cleardrawingwand($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearmagickwand($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearpixeliterator($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearpixelwand($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clonedrawingwand($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clonemagickwand($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexception($wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexceptionstring($wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexceptiontype($wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandhasexception($wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawaffine(
  $drw_wnd,
  $sx,
  $sy,
  $rx,
  $ry,
  $tx,
  $ty,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawannotation($drw_wnd, $x, $y, $text): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawarc(
  $drw_wnd,
  $sx,
  $sy,
  $ex,
  $ey,
  $sd,
  $ed,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawbezier($drw_wnd, $x_y_points_array): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcircle($drw_wnd, $ox, $oy, $px, $py): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcolor(
  $drw_wnd,
  $x,
  $y,
  $paint_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcomment($drw_wnd, $comment): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcomposite(
  $drw_wnd,
  $composite_operator,
  $x,
  $y,
  $width,
  $height,
  $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawellipse(
  $drw_wnd,
  $ox,
  $oy,
  $rx,
  $ry,
  $start,
  $end,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetclippath($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetcliprule($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetclipunits($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexception($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexceptionstring($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexceptiontype($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillalpha($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillcolor($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillopacity($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillrule($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfont($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontfamily($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontsize($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontstretch($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontstyle($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontweight($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetgravity($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokealpha($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokeantialias($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokecolor($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokedasharray($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokedashoffset($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokelinecap($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokelinejoin($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokemiterlimit($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokeopacity($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokewidth($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextalignment($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextantialias($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextdecoration($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextencoding($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextundercolor($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetvectorgraphics($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawline($drw_wnd, $sx, $sy, $ex, $ey): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawmatte(
  $drw_wnd,
  $x,
  $y,
  $paint_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathclose($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoabsolute(
  $drw_wnd,
  $x1,
  $y1,
  $x2,
  $y2,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierabsolute(
  $drw_wnd,
  $x1,
  $y1,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierrelative(
  $drw_wnd,
  $x1,
  $y1,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothabsolute(
  $drw_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothrelative(
  $drw_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetorelative(
  $drw_wnd,
  $x1,
  $y1,
  $x2,
  $y2,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetosmoothabsolute(
  $drw_wnd,
  $x2,
  $y2,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetosmoothrelative(
  $drw_wnd,
  $x2,
  $y2,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathellipticarcabsolute(
  $drw_wnd,
  $rx,
  $ry,
  $x_axis_rotation,
  $large_arc_flag,
  $sweep_flag,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathellipticarcrelative(
  $drw_wnd,
  $rx,
  $ry,
  $x_axis_rotation,
  $large_arc_flag,
  $sweep_flag,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathfinish($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoabsolute($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetohorizontalabsolute(
  $drw_wnd,
  $x,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetohorizontalrelative(
  $drw_wnd,
  $x,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetorelative($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoverticalabsolute(
  $drw_wnd,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoverticalrelative(
  $drw_wnd,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathmovetoabsolute($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathmovetorelative($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathstart($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpoint($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpolygon($drw_wnd, $x_y_points_array): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpolyline(
  $drw_wnd,
  $x_y_points_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrectangle(
  $drw_wnd,
  $x1,
  $y1,
  $x2,
  $y2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrender($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrotate($drw_wnd, $degrees): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawroundrectangle(
  $drw_wnd,
  $x1,
  $y1,
  $x2,
  $y2,
  $rx,
  $ry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawscale($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetclippath($drw_wnd, $clip_path): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetcliprule($drw_wnd, $fill_rule): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetclipunits(
  $drw_wnd,
  $clip_path_units,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillalpha(
  $drw_wnd,
  $fill_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillcolor(
  $drw_wnd,
  $fill_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillopacity(
  $drw_wnd,
  $fill_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillpatternurl(
  $drw_wnd,
  $fill_url,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillrule($drw_wnd, $fill_rule): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfont($drw_wnd, $font_file): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontfamily(
  $drw_wnd,
  $font_family,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontsize($drw_wnd, $pointsize): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontstretch(
  $drw_wnd,
  $stretch_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontstyle($drw_wnd, $style_type): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontweight(
  $drw_wnd,
  $font_weight,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetgravity($drw_wnd, $gravity_type): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokealpha(
  $drw_wnd,
  $stroke_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokeantialias(
  $drw_wnd,
  $stroke_antialias = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokecolor(
  $drw_wnd,
  $strokecolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokedasharray(
  $drw_wnd,
  $dash_array = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokedashoffset(
  $drw_wnd,
  $dash_offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokelinecap(
  $drw_wnd,
  $line_cap,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokelinejoin(
  $drw_wnd,
  $line_join,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokemiterlimit(
  $drw_wnd,
  $miterlimit,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokeopacity(
  $drw_wnd,
  $stroke_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokepatternurl(
  $drw_wnd,
  $stroke_url,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokewidth(
  $drw_wnd,
  $stroke_width,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextalignment(
  $drw_wnd,
  $align_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextantialias(
  $drw_wnd,
  $text_antialias = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextdecoration(
  $drw_wnd,
  $decoration_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextencoding($drw_wnd, $encoding): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextundercolor(
  $drw_wnd,
  $undercolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetvectorgraphics(
  $drw_wnd,
  $vector_graphics,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetviewbox(
  $drw_wnd,
  $x1,
  $y1,
  $x2,
  $y2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawskewx($drw_wnd, $degrees): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawskewy($drw_wnd, $degrees): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawtranslate($drw_wnd, $x, $y): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pushdrawingwand($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushclippath(
  $drw_wnd,
  $clip_path_id,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushdefs($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushpattern(
  $drw_wnd,
  $pattern_id,
  $x,
  $y,
  $width,
  $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function popdrawingwand($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpopclippath($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpopdefs($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpoppattern($drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickadaptivethresholdimage(
  $mgck_wnd,
  $width,
  $height,
  $offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaddimage($mgck_wnd, $add_wand): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaddnoiseimage(
  $mgck_wnd,
  $noise_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaffinetransformimage(
  $mgck_wnd,
  $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickannotateimage(
  $mgck_wnd,
  $drw_wnd,
  $x,
  $y,
  $angle,
  $text,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickappendimages(
  $mgck_wnd,
  $stack_vertical = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaverageimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickblackthresholdimage(
  $mgck_wnd,
  $threshold_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickblurimage(
  $mgck_wnd,
  $radius,
  $sigma,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickborderimage(
  $mgck_wnd,
  $bordercolor,
  $width,
  $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcharcoalimage(
  $mgck_wnd,
  $radius,
  $sigma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickchopimage(
  $mgck_wnd,
  $width,
  $height,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickclipimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickclippathimage(
  $mgck_wnd,
  $pathname,
  $inside,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcoalesceimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcolorfloodfillimage(
  $mgck_wnd,
  $fillcolor_pxl_wnd,
  $fuzz,
  $bordercolor_pxl_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcolorizeimage(
  $mgck_wnd,
  $colorize,
  $opacity_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcombineimages(
  $mgck_wnd,
  $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcommentimage($mgck_wnd, $comment): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcompareimages(
  $mgck_wnd,
  $reference_wnd,
  $metric_type,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcompositeimage(
  $mgck_wnd,
  $composite_wnd,
  $composite_operator,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickconstituteimage(
  $mgck_wnd,
  $columns,
  $rows,
  $smap,
  $storage_type,
  $pixel_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcontrastimage($mgck_wnd, $sharpen): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickconvolveimage(
  $mgck_wnd,
  $kernel_array,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcropimage(
  $mgck_wnd,
  $width,
  $height,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcyclecolormapimage(
  $mgck_wnd,
  $num_positions,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdeconstructimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdescribeimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdespeckleimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdrawimage($mgck_wnd, $drw_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickechoimageblob($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickechoimagesblob($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickedgeimage($mgck_wnd, $radius): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickembossimage(
  $mgck_wnd,
  $radius,
  $sigma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickenhanceimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickequalizeimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickevaluateimage(
  $mgck_wnd,
  $evaluate_op,
  $constant,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflattenimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflipimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflopimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickframeimage(
  $mgck_wnd,
  $matte_color,
  $width,
  $height,
  $inner_bevel,
  $outer_bevel,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickfximage(
  $mgck_wnd,
  $expression,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgammaimage(
  $mgck_wnd,
  $gamma,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgaussianblurimage(
  $mgck_wnd,
  $radius,
  $sigma,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetcharheight(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetcharwidth(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexception($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexceptionstring($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexceptiontype($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetfilename($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetformat($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagebackgroundcolor($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageblob($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageblueprimary($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagebordercolor($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagechannelmean(
  $mgck_wnd,
  $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolormapcolor(
  $mgck_wnd,
  $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolors($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolorspace($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompose($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompression($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompressionquality(
  $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedelay($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedepth(
  $mgck_wnd,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedispose($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageextrema(
  $mgck_wnd,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagefilename($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageformat($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagegamma($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagegreenprimary($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageheight($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagehistogram($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageindex($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageinterlacescheme($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageiterations($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagemattecolor($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagemimetype($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagepixels(
  $mgck_wnd,
  $x_offset,
  $y_offset,
  $columns,
  $rows,
  $smap,
  $storage_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageprofile($mgck_wnd, $name): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageredprimary($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagerenderingintent($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageresolution($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagescene($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesignature($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesize($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagetype($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageunits($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagevirtualpixelmethod(
  $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagewhitepoint($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagewidth($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesblob($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetinterlacescheme($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetmaxtextadvance(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetmimetype($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetnumberimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetsamplingfactors($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetsize($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetstringheight(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetstringwidth(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgettextascent(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgettextdescent(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetwandsize($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickhasnextimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickhaspreviousimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickimplodeimage($mgck_wnd, $amount): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicklabelimage($mgck_wnd, $label): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicklevelimage(
  $mgck_wnd,
  $black_point,
  $gamma,
  $white_point,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmagnifyimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmapimage(
  $mgck_wnd,
  $map_wand,
  $dither,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmattefloodfillimage(
  $mgck_wnd,
  $opacity,
  $fuzz,
  $bordercolor_pxl_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmedianfilterimage(
  $mgck_wnd,
  $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickminifyimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmodulateimage(
  $mgck_wnd,
  $brightness,
  $saturation,
  $hue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmontageimage(
  $mgck_wnd,
  $drw_wnd,
  $tile_geometry,
  $thumbnail_geometry,
  $montage_mode,
  $frame,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmorphimages(
  $mgck_wnd,
  $number_frames,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmosaicimages($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmotionblurimage(
  $mgck_wnd,
  $radius,
  $sigma,
  $angle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknegateimage(
  $mgck_wnd,
  $only_the_gray = false,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknewimage(
  $mgck_wnd,
  $width,
  $height,
  $imagemagick_col_str = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknextimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknormalizeimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickoilpaintimage($mgck_wnd, $radius): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpaintopaqueimage(
  $mgck_wnd,
  $target_pxl_wnd,
  $fill_pxl_wnd,
  $fuzz = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpainttransparentimage(
  $mgck_wnd,
  $target,
  $opacity = MW_TransparentOpacity,
  $fuzz = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpingimage($mgck_wnd, $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickposterizeimage(
  $mgck_wnd,
  $levels,
  $dither,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpreviewimages($mgck_wnd, $preview): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpreviousimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickprofileimage(
  $mgck_wnd,
  $name,
  $profile = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickquantizeimage(
  $mgck_wnd,
  $number_colors,
  $colorspace_type,
  $treedepth,
  $dither,
  $measure_error,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickquantizeimages(
  $mgck_wnd,
  $number_colors,
  $colorspace_type,
  $treedepth,
  $dither,
  $measure_error,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryfontmetrics(
  $mgck_wnd,
  $drw_wnd,
  $txt,
  $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickradialblurimage($mgck_wnd, $angle): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickraiseimage(
  $mgck_wnd,
  $width,
  $height,
  $x,
  $y,
  $raise,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimage($mgck_wnd, $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimageblob($mgck_wnd, $blob): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimagefile($mgck_wnd, $handle): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimages(
  $mgck_wnd,
  $img_filenames_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreducenoiseimage(
  $mgck_wnd,
  $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimageprofile(
  $mgck_wnd,
  $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimageprofiles($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresampleimage(
  $mgck_wnd,
  $x_resolution,
  $y_resolution,
  $filter_type,
  $blur,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresetiterator($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresizeimage(
  $mgck_wnd,
  $columns,
  $rows,
  $filter_type,
  $blur,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickrollimage(
  $mgck_wnd,
  $x_offset,
  $y_offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickrotateimage(
  $mgck_wnd,
  $background,
  $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksampleimage(
  $mgck_wnd,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickscaleimage(
  $mgck_wnd,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickseparateimagechannel(
  $mgck_wnd,
  $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetcompressionquality(
  $mgck_wnd,
  $quality,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetfilename(
  $mgck_wnd,
  $filename = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetfirstiterator($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetformat($mgck_wnd, $format): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimage($mgck_wnd, $replace_wand): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebackgroundcolor(
  $mgck_wnd,
  $background_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebias($mgck_wnd, $bias): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageblueprimary(
  $mgck_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebordercolor(
  $mgck_wnd,
  $border_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecolormapcolor(
  $mgck_wnd,
  $index,
  $mapcolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecolorspace(
  $mgck_wnd,
  $colorspace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompose(
  $mgck_wnd,
  $composite_operator,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompression(
  $mgck_wnd,
  $compression_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompressionquality(
  $mgck_wnd,
  $quality,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedelay($mgck_wnd, $delay): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedepth(
  $mgck_wnd,
  $depth,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedispose(
  $mgck_wnd,
  $dispose_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagefilename(
  $mgck_wnd,
  $filename = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageformat($mgck_wnd, $format): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagegamma($mgck_wnd, $gamma): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagegreenprimary(
  $mgck_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageindex($mgck_wnd, $index): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageinterlacescheme(
  $mgck_wnd,
  $interlace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageiterations(
  $mgck_wnd,
  $iterations,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagemattecolor(
  $mgck_wnd,
  $matte_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageoption(
  $mgck_wnd,
  $format,
  $key,
  $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagepixels(
  $mgck_wnd,
  $x_offset,
  $y_offset,
  $columns,
  $rows,
  $smap,
  $storage_type,
  $pixel_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageprofile(
  $mgck_wnd,
  $name,
  $profile,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageredprimary(
  $mgck_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagerenderingintent(
  $mgck_wnd,
  $rendering_intent,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageresolution(
  $mgck_wnd,
  $x_resolution,
  $y_resolution,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagescene($mgck_wnd, $scene): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagetype(
  $mgck_wnd,
  $image_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageunits(
  $mgck_wnd,
  $resolution_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagevirtualpixelmethod(
  $mgck_wnd,
  $virtual_pixel_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagewhitepoint(
  $mgck_wnd,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetinterlacescheme(
  $mgck_wnd,
  $interlace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetlastiterator($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetpassphrase(
  $mgck_wnd,
  $passphrase,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetresolution(
  $mgck_wnd,
  $x_resolution,
  $y_resolution,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetsamplingfactors(
  $mgck_wnd,
  $number_factors,
  $sampling_factors,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetsize(
  $mgck_wnd,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetwandsize(
  $mgck_wnd,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksharpenimage(
  $mgck_wnd,
  $radius,
  $sigma,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickshaveimage(
  $mgck_wnd,
  $columns,
  $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickshearimage(
  $mgck_wnd,
  $background,
  $x_shear,
  $y_shear,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksolarizeimage(
  $mgck_wnd,
  $threshold,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickspliceimage(
  $mgck_wnd,
  $width,
  $height,
  $x,
  $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickspreadimage($mgck_wnd, $radius): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksteganoimage(
  $mgck_wnd,
  $watermark_wand,
  $offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickstereoimage(
  $mgck_wnd,
  $offset_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickstripimage($mgck_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickswirlimage($mgck_wnd, $degrees): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktextureimage(
  $mgck_wnd,
  $texture_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickthresholdimage(
  $mgck_wnd,
  $threshold,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktintimage(
  $mgck_wnd,
  $tint_pxl_wnd,
  $opacity_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktransformimage(
  $mgck_wnd,
  $crop,
  $geometry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktrimimage($mgck_wnd, $fuzz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickunsharpmaskimage(
  $mgck_wnd,
  $radius,
  $sigma,
  $amount,
  $threshold,
  $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwaveimage(
  $mgck_wnd,
  $amplitude,
  $wave_length,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwhitethresholdimage(
  $mgck_wnd,
  $threshold_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimage($mgck_wnd, $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimagefile($mgck_wnd, $handle): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimages(
  $mgck_wnd,
  $filename = "",
  $join_images = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimagesfile(
  $mgck_wnd,
  $handle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetalpha($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetalphaquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblack($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblackquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblue($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetbluequantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcolorasstring($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcolorcount($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcyan($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcyanquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexception($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexceptionstring($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexceptiontype($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetgreen($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetgreenquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetindex($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetmagenta($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetmagentaquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetopacity($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetopacityquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetquantumcolor($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetred($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetredquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetyellow($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetyellowquantum($pxl_wnd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetalpha($pxl_wnd, $alpha): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetalphaquantum($pxl_wnd, $alpha): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblack($pxl_wnd, $black): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblackquantum($pxl_wnd, $black): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblue($pxl_wnd, $blue): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetbluequantum($pxl_wnd, $blue): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcolor(
  $pxl_wnd,
  $imagemagick_col_str,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcolorcount($pxl_wnd, $count): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcyan($pxl_wnd, $cyan): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcyanquantum($pxl_wnd, $cyan): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetgreen($pxl_wnd, $green): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetgreenquantum($pxl_wnd, $green): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetindex($pxl_wnd, $index): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetmagenta($pxl_wnd, $magenta): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetmagentaquantum(
  $pxl_wnd,
  $magenta,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetopacity($pxl_wnd, $opacity): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetopacityquantum(
  $pxl_wnd,
  $opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetquantumcolor(
  $pxl_wnd,
  $red,
  $green,
  $blue,
  $opacity = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetred($pxl_wnd, $red): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetredquantum($pxl_wnd, $red): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetyellow($pxl_wnd, $yellow): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetyellowquantum($pxl_wnd, $yellow): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexception($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexceptionstring(
  $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexceptiontype($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetnextiteratorrow($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelresetiterator($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetiteratorrow($pxl_iter, $row): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsynciterator($pxl_iter): HH\FIXME\MISSING_RETURN_TYPE;
