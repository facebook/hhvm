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
function magickgetresourcelimit(
  HH\FIXME\MISSING_PARAM_TYPE $resource_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversion(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversionnumber(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetversionstring(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryconfigureoption(
  HH\FIXME\MISSING_PARAM_TYPE $option,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryconfigureoptions(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryfonts(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryformats(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetresourcelimit(
  HH\FIXME\MISSING_PARAM_TYPE $resource_type,
  HH\FIXME\MISSING_PARAM_TYPE $limit,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newdrawingwand(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newmagickwand(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixeliterator(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelregioniterator(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwand(
  HH\FIXME\MISSING_PARAM_TYPE $imagemagick_col_str = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwandarray(
  HH\FIXME\MISSING_PARAM_TYPE $num_pxl_wnds,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function newpixelwands(
  HH\FIXME\MISSING_PARAM_TYPE $num_pxl_wnds,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroydrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroymagickwand(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixeliterator(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwand(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwandarray(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function destroypixelwands(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function isdrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ismagickwand(
  HH\FIXME\MISSING_PARAM_TYPE $var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ispixeliterator(
  HH\FIXME\MISSING_PARAM_TYPE $var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ispixelwand(
  HH\FIXME\MISSING_PARAM_TYPE $var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function cleardrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearmagickwand(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearpixeliterator(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clearpixelwand(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clonedrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clonemagickwand(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexception(
  HH\FIXME\MISSING_PARAM_TYPE $wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexceptionstring(
  HH\FIXME\MISSING_PARAM_TYPE $wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandgetexceptiontype(
  HH\FIXME\MISSING_PARAM_TYPE $wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wandhasexception(
  HH\FIXME\MISSING_PARAM_TYPE $wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawaffine(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $sx,
  HH\FIXME\MISSING_PARAM_TYPE $sy,
  HH\FIXME\MISSING_PARAM_TYPE $rx,
  HH\FIXME\MISSING_PARAM_TYPE $ry,
  HH\FIXME\MISSING_PARAM_TYPE $tx,
  HH\FIXME\MISSING_PARAM_TYPE $ty,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawannotation(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $text,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawarc(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $sx,
  HH\FIXME\MISSING_PARAM_TYPE $sy,
  HH\FIXME\MISSING_PARAM_TYPE $ex,
  HH\FIXME\MISSING_PARAM_TYPE $ey,
  HH\FIXME\MISSING_PARAM_TYPE $sd,
  HH\FIXME\MISSING_PARAM_TYPE $ed,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawbezier(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_y_points_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcircle(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $ox,
  HH\FIXME\MISSING_PARAM_TYPE $oy,
  HH\FIXME\MISSING_PARAM_TYPE $px,
  HH\FIXME\MISSING_PARAM_TYPE $py,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $paint_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcomment(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $comment,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawcomposite(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $composite_operator,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawellipse(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $ox,
  HH\FIXME\MISSING_PARAM_TYPE $oy,
  HH\FIXME\MISSING_PARAM_TYPE $rx,
  HH\FIXME\MISSING_PARAM_TYPE $ry,
  HH\FIXME\MISSING_PARAM_TYPE $start,
  HH\FIXME\MISSING_PARAM_TYPE $end,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetclippath(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetcliprule(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetclipunits(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexception(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexceptionstring(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetexceptiontype(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillalpha(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillcolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillopacity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfillrule(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfont(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontfamily(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontsize(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontstretch(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontstyle(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetfontweight(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetgravity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokealpha(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokeantialias(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokecolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokedasharray(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokedashoffset(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokelinecap(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokelinejoin(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokemiterlimit(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokeopacity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetstrokewidth(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextalignment(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextantialias(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextdecoration(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextencoding(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgettextundercolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawgetvectorgraphics(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawline(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $sx,
  HH\FIXME\MISSING_PARAM_TYPE $sy,
  HH\FIXME\MISSING_PARAM_TYPE $ex,
  HH\FIXME\MISSING_PARAM_TYPE $ey,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawmatte(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $paint_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathclose(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetorelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetosmoothabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathcurvetosmoothrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathellipticarcabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $rx,
  HH\FIXME\MISSING_PARAM_TYPE $ry,
  HH\FIXME\MISSING_PARAM_TYPE $x_axis_rotation,
  HH\FIXME\MISSING_PARAM_TYPE $large_arc_flag,
  HH\FIXME\MISSING_PARAM_TYPE $sweep_flag,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathellipticarcrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $rx,
  HH\FIXME\MISSING_PARAM_TYPE $ry,
  HH\FIXME\MISSING_PARAM_TYPE $x_axis_rotation,
  HH\FIXME\MISSING_PARAM_TYPE $large_arc_flag,
  HH\FIXME\MISSING_PARAM_TYPE $sweep_flag,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathfinish(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetohorizontalabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetohorizontalrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetorelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoverticalabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathlinetoverticalrelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathmovetoabsolute(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathmovetorelative(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpathstart(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpoint(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpolygon(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_y_points_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpolyline(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_y_points_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrectangle(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrender(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawrotate(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawroundrectangle(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
  HH\FIXME\MISSING_PARAM_TYPE $rx,
  HH\FIXME\MISSING_PARAM_TYPE $ry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawscale(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetclippath(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $clip_path,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetcliprule(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_rule,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetclipunits(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $clip_path_units,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillalpha(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillcolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillopacity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillpatternurl(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_url,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfillrule(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_rule,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfont(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $font_file,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontfamily(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $font_family,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontsize(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $pointsize,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontstretch(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stretch_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontstyle(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $style_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetfontweight(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $font_weight,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetgravity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $gravity_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokealpha(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stroke_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokeantialias(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stroke_antialias = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokecolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $strokecolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokedasharray(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $dash_array = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokedashoffset(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $dash_offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokelinecap(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $line_cap,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokelinejoin(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $line_join,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokemiterlimit(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $miterlimit,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokeopacity(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stroke_opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokepatternurl(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stroke_url,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetstrokewidth(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stroke_width,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextalignment(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $align_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextantialias(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $text_antialias = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextdecoration(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $decoration_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextencoding(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $encoding,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsettextundercolor(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $undercolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetvectorgraphics(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $vector_graphics,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawsetviewbox(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x1,
  HH\FIXME\MISSING_PARAM_TYPE $y1,
  HH\FIXME\MISSING_PARAM_TYPE $x2,
  HH\FIXME\MISSING_PARAM_TYPE $y2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawskewx(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawskewy(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawtranslate(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pushdrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushclippath(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $clip_path_id,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushdefs(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpushpattern(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $pattern_id,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function popdrawingwand(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpopclippath(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpopdefs(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function drawpoppattern(
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickadaptivethresholdimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaddimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $add_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaddnoiseimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $noise_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaffinetransformimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickannotateimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $angle,
  HH\FIXME\MISSING_PARAM_TYPE $text,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickappendimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $stack_vertical = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickaverageimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickblackthresholdimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $threshold_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickblurimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickborderimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $bordercolor,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcharcoalimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickchopimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickclipimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickclippathimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $pathname,
  HH\FIXME\MISSING_PARAM_TYPE $inside,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcoalesceimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcolorfloodfillimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fillcolor_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fuzz,
  HH\FIXME\MISSING_PARAM_TYPE $bordercolor_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcolorizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $colorize,
  HH\FIXME\MISSING_PARAM_TYPE $opacity_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcombineimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcommentimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $comment,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcompareimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $reference_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $metric_type,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcompositeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $composite_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $composite_operator,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickconstituteimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
  HH\FIXME\MISSING_PARAM_TYPE $smap,
  HH\FIXME\MISSING_PARAM_TYPE $storage_type,
  HH\FIXME\MISSING_PARAM_TYPE $pixel_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcontrastimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $sharpen,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickconvolveimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $kernel_array,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcropimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickcyclecolormapimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $num_positions,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdeconstructimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdescribeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdespeckleimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickdrawimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickechoimageblob(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickechoimagesblob(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickedgeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickembossimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickenhanceimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickequalizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickevaluateimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $evaluate_op,
  HH\FIXME\MISSING_PARAM_TYPE $constant,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflattenimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflipimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickflopimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickframeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $matte_color,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $inner_bevel,
  HH\FIXME\MISSING_PARAM_TYPE $outer_bevel,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickfximage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $expression,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgammaimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $gamma,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgaussianblurimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetcharheight(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetcharwidth(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexception(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexceptionstring(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetexceptiontype(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetfilename(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetformat(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagebackgroundcolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageblob(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageblueprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagebordercolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagechannelmean(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolormapcolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolors(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecolorspace(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompose(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompression(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagecompressionquality(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedelay(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedepth(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagedispose(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageextrema(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagefilename(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageformat(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagegamma(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagegreenprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageheight(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagehistogram(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageindex(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageinterlacescheme(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageiterations(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagemattecolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagemimetype(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagepixels(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_offset,
  HH\FIXME\MISSING_PARAM_TYPE $y_offset,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
  HH\FIXME\MISSING_PARAM_TYPE $smap,
  HH\FIXME\MISSING_PARAM_TYPE $storage_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageprofile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageredprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagerenderingintent(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageresolution(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagescene(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesignature(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesize(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagetype(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimageunits(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagevirtualpixelmethod(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagewhitepoint(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagewidth(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetimagesblob(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetinterlacescheme(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetmaxtextadvance(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetmimetype(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetnumberimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetsamplingfactors(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetsize(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetstringheight(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetstringwidth(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgettextascent(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgettextdescent(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickgetwandsize(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickhasnextimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickhaspreviousimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickimplodeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $amount,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicklabelimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $label,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicklevelimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $black_point,
  HH\FIXME\MISSING_PARAM_TYPE $gamma,
  HH\FIXME\MISSING_PARAM_TYPE $white_point,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmagnifyimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmapimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $map_wand,
  HH\FIXME\MISSING_PARAM_TYPE $dither,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmattefloodfillimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $opacity,
  HH\FIXME\MISSING_PARAM_TYPE $fuzz,
  HH\FIXME\MISSING_PARAM_TYPE $bordercolor_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmedianfilterimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickminifyimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmodulateimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $brightness,
  HH\FIXME\MISSING_PARAM_TYPE $saturation,
  HH\FIXME\MISSING_PARAM_TYPE $hue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmontageimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $tile_geometry,
  HH\FIXME\MISSING_PARAM_TYPE $thumbnail_geometry,
  HH\FIXME\MISSING_PARAM_TYPE $montage_mode,
  HH\FIXME\MISSING_PARAM_TYPE $frame,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmorphimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $number_frames,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmosaicimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickmotionblurimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
  HH\FIXME\MISSING_PARAM_TYPE $angle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknegateimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $only_the_gray = false,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknewimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $imagemagick_col_str = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknextimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicknormalizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickoilpaintimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpaintopaqueimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $target_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fill_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fuzz = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpainttransparentimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $target,
  HH\FIXME\MISSING_PARAM_TYPE $opacity = MW_TransparentOpacity,
  HH\FIXME\MISSING_PARAM_TYPE $fuzz = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpingimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickposterizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $levels,
  HH\FIXME\MISSING_PARAM_TYPE $dither,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpreviewimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $preview,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickpreviousimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickprofileimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $name,
  HH\FIXME\MISSING_PARAM_TYPE $profile = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickquantizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $number_colors,
  HH\FIXME\MISSING_PARAM_TYPE $colorspace_type,
  HH\FIXME\MISSING_PARAM_TYPE $treedepth,
  HH\FIXME\MISSING_PARAM_TYPE $dither,
  HH\FIXME\MISSING_PARAM_TYPE $measure_error,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickquantizeimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $number_colors,
  HH\FIXME\MISSING_PARAM_TYPE $colorspace_type,
  HH\FIXME\MISSING_PARAM_TYPE $treedepth,
  HH\FIXME\MISSING_PARAM_TYPE $dither,
  HH\FIXME\MISSING_PARAM_TYPE $measure_error,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickqueryfontmetrics(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $drw_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $txt,
  HH\FIXME\MISSING_PARAM_TYPE $multiline = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickradialblurimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $angle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickraiseimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
  HH\FIXME\MISSING_PARAM_TYPE $raise,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimageblob(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $blob,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimagefile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $handle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreadimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $img_filenames_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickreducenoiseimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimageprofile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickremoveimageprofiles(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresampleimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_resolution,
  HH\FIXME\MISSING_PARAM_TYPE $y_resolution,
  HH\FIXME\MISSING_PARAM_TYPE $filter_type,
  HH\FIXME\MISSING_PARAM_TYPE $blur,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresetiterator(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickresizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
  HH\FIXME\MISSING_PARAM_TYPE $filter_type,
  HH\FIXME\MISSING_PARAM_TYPE $blur,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickrollimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_offset,
  HH\FIXME\MISSING_PARAM_TYPE $y_offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickrotateimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $background,
  HH\FIXME\MISSING_PARAM_TYPE $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksampleimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickscaleimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickseparateimagechannel(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetcompressionquality(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $quality,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetfilename(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetfirstiterator(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetformat(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $format,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $replace_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebackgroundcolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $background_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebias(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $bias,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageblueprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagebordercolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $border_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecolormapcolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $index,
  HH\FIXME\MISSING_PARAM_TYPE $mapcolor_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecolorspace(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $colorspace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompose(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $composite_operator,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompression(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $compression_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagecompressionquality(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $quality,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedelay(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $delay,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedepth(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $depth,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagedispose(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $dispose_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagefilename(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageformat(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $format,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagegamma(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $gamma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagegreenprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageindex(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageinterlacescheme(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $interlace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageiterations(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $iterations,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagemattecolor(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $matte_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageoption(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $format,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagepixels(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_offset,
  HH\FIXME\MISSING_PARAM_TYPE $y_offset,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
  HH\FIXME\MISSING_PARAM_TYPE $smap,
  HH\FIXME\MISSING_PARAM_TYPE $storage_type,
  HH\FIXME\MISSING_PARAM_TYPE $pixel_array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageprofile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $name,
  HH\FIXME\MISSING_PARAM_TYPE $profile,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageredprimary(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagerenderingintent(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $rendering_intent,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageresolution(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_resolution,
  HH\FIXME\MISSING_PARAM_TYPE $y_resolution,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagescene(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $scene,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagetype(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $image_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimageunits(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $resolution_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagevirtualpixelmethod(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $virtual_pixel_method,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetimagewhitepoint(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetinterlacescheme(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $interlace_type,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetlastiterator(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetpassphrase(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $passphrase,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetresolution(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $x_resolution,
  HH\FIXME\MISSING_PARAM_TYPE $y_resolution,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetsamplingfactors(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $number_factors,
  HH\FIXME\MISSING_PARAM_TYPE $sampling_factors,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetsize(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksetwandsize(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksharpenimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickshaveimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $columns,
  HH\FIXME\MISSING_PARAM_TYPE $rows,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickshearimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $background,
  HH\FIXME\MISSING_PARAM_TYPE $x_shear,
  HH\FIXME\MISSING_PARAM_TYPE $y_shear,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksolarizeimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $threshold,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickspliceimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $width,
  HH\FIXME\MISSING_PARAM_TYPE $height,
  HH\FIXME\MISSING_PARAM_TYPE $x,
  HH\FIXME\MISSING_PARAM_TYPE $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickspreadimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicksteganoimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $watermark_wand,
  HH\FIXME\MISSING_PARAM_TYPE $offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickstereoimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $offset_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickstripimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickswirlimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $degrees,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktextureimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $texture_wand,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickthresholdimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $threshold,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktintimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $tint_pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $opacity_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktransformimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $crop,
  HH\FIXME\MISSING_PARAM_TYPE $geometry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magicktrimimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $fuzz,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickunsharpmaskimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $radius,
  HH\FIXME\MISSING_PARAM_TYPE $sigma,
  HH\FIXME\MISSING_PARAM_TYPE $amount,
  HH\FIXME\MISSING_PARAM_TYPE $threshold,
  HH\FIXME\MISSING_PARAM_TYPE $channel_type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwaveimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $amplitude,
  HH\FIXME\MISSING_PARAM_TYPE $wave_length,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwhitethresholdimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $threshold_pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimage(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimagefile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $handle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimages(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $filename = "",
  HH\FIXME\MISSING_PARAM_TYPE $join_images = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function magickwriteimagesfile(
  HH\FIXME\MISSING_PARAM_TYPE $mgck_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $handle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetalpha(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetalphaquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblack(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblackquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetblue(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetbluequantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcolorasstring(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcolorcount(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcyan(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetcyanquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexception(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexceptionstring(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetexceptiontype(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetgreen(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetgreenquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetindex(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetmagenta(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetmagentaquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetopacity(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetopacityquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetquantumcolor(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetred(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetredquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetyellow(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetyellowquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetalpha(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetalphaquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblack(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $black,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblackquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $black,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetblue(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetbluequantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcolor(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $imagemagick_col_str,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcolorcount(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $count,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcyan(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $cyan,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetcyanquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $cyan,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetgreen(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $green,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetgreenquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $green,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetindex(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetmagenta(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $magenta,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetmagentaquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $magenta,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetopacity(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetopacityquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $opacity,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetquantumcolor(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $red,
  HH\FIXME\MISSING_PARAM_TYPE $green,
  HH\FIXME\MISSING_PARAM_TYPE $blue,
  HH\FIXME\MISSING_PARAM_TYPE $opacity = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetred(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $red,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetredquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $red,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetyellow(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $yellow,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetyellowquantum(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_wnd,
  HH\FIXME\MISSING_PARAM_TYPE $yellow,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexception(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexceptionstring(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetiteratorexceptiontype(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelgetnextiteratorrow(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelresetiterator(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsetiteratorrow(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
  HH\FIXME\MISSING_PARAM_TYPE $row,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pixelsynciterator(
  HH\FIXME\MISSING_PARAM_TYPE $pxl_iter,
): HH\FIXME\MISSING_RETURN_TYPE;
