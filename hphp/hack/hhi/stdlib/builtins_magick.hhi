<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MW_AbsoluteIntent = 3;
const int MW_AddCompositeOp = 2;
const int MW_AddEvaluateOperator = 1;
const int MW_AddNoisePreview = 14;
const int MW_AllChannels = 255;
const int MW_AlphaChannel = 8;
const int MW_AndEvaluateOperator = 2;
const int MW_AnyStretch = 10;
const int MW_AnyStyle = 4;
const int MW_AreaResource = 1;
const int MW_AtopCompositeOp = 3;
const int MW_BZipCompression = 2;
const int MW_BackgroundDispose = 2;
const int MW_BesselFilter = 14;
const int MW_BevelJoin = 3;
const int MW_BilevelType = 1;
const int MW_BlackChannel = 32;
const int MW_BlackmanFilter = 7;
const int MW_BlendCompositeOp = 4;
const int MW_BlobError = 435;
const int MW_BlobFatalError = 735;
const int MW_BlobWarning = 335;
const int MW_BlueChannel = 4;
const int MW_BlurPreview = 16;
const int MW_BoxFilter = 2;
const int MW_BrightnessPreview = 6;
const int MW_BumpmapCompositeOp = 5;
const int MW_ButtCap = 1;
const int MW_CMYKColorspace = 12;
const int MW_CacheError = 445;
const int MW_CacheFatalError = 745;
const int MW_CacheWarning = 345;
const int MW_CatromFilter = 11;
const int MW_CenterAlign = 2;
const int MW_CenterGravity = 5;
const int MW_CharPixel = 1;
const int MW_CharcoalDrawingPreview = 28;
const int MW_ClearCompositeOp = 7;
const int MW_CoderError = 450;
const int MW_CoderFatalError = 750;
const int MW_CoderWarning = 350;
const int MW_ColorBurnCompositeOp = 8;
const int MW_ColorDodgeCompositeOp = 9;
const int MW_ColorSeparationMatteType = 9;
const int MW_ColorSeparationType = 8;
const int MW_ColorizeCompositeOp = 10;
const int MW_ConcatenateMode = 3;
const int MW_CondensedStretch = 4;
const int MW_ConfigureError = 495;
const int MW_ConfigureFatalError = 795;
const int MW_ConfigureWarning = 395;
const int MW_ConstantVirtualPixelMethod = 2;
const int MW_CopyBlackCompositeOp = 11;
const int MW_CopyBlueCompositeOp = 12;
const int MW_CopyCompositeOp = 13;
const int MW_CopyCyanCompositeOp = 14;
const int MW_CopyGreenCompositeOp = 15;
const int MW_CopyMagentaCompositeOp = 16;
const int MW_CopyOpacityCompositeOp = 17;
const int MW_CopyRedCompositeOp = 18;
const int MW_CopyYellowCompositeOp = 19;
const int MW_CorruptImageError = 425;
const int MW_CorruptImageFatalError = 725;
const int MW_CorruptImageWarning = 325;
const int MW_CubicFilter = 10;
const int MW_CyanChannel = 1;
const int MW_DarkenCompositeOp = 20;
const int MW_DelegateError = 415;
const int MW_DelegateFatalError = 715;
const int MW_DelegateWarning = 315;
const int MW_DespecklePreview = 12;
const int MW_DifferenceCompositeOp = 26;
const int MW_DiskResource = 2;
const int MW_DisplaceCompositeOp = 27;
const int MW_DissolveCompositeOp = 28;
const int MW_DivideEvaluateOperator = 3;
const int MW_DoublePixel = 2;
const int MW_DrawError = 460;
const int MW_DrawFatalError = 760;
const int MW_DrawWarning = 360;
const int MW_DstAtopCompositeOp = 21;
const int MW_DstCompositeOp = 22;
const int MW_DstInCompositeOp = 23;
const int MW_DstOutCompositeOp = 24;
const int MW_DstOverCompositeOp = 25;
const int MW_DullPreview = 9;
const int MW_EastGravity = 6;
const int MW_EdgeDetectPreview = 18;
const int MW_EdgeVirtualPixelMethod = 4;
const int MW_ErrorException = 400;
const int MW_EvenOddRule = 1;
const int MW_ExclusionCompositeOp = 29;
const int MW_ExpandedStretch = 7;
const int MW_ExtraCondensedStretch = 3;
const int MW_ExtraExpandedStretch = 8;
const int MW_FatalErrorException = 700;
const int MW_FaxCompression = 3;
const int MW_FileOpenError = 430;
const int MW_FileOpenFatalError = 730;
const int MW_FileOpenWarning = 330;
const int MW_FileResource = 3;
const int MW_FillToBorderMethod = 4;
const int MW_FloatPixel = 3;
const int MW_FloodfillMethod = 3;
const int MW_ForgetGravity = 0;
const int MW_FrameMode = 1;
const int MW_GRAYColorspace = 2;
const int MW_GammaPreview = 7;
const int MW_GaussianFilter = 8;
const int MW_GaussianNoise = 2;
const int MW_GrayscaleMatteType = 3;
const int MW_GrayscalePreview = 10;
const int MW_GrayscaleType = 2;
const int MW_GreenChannel = 2;
const int MW_Group4Compression = 4;
const int MW_HSBColorspace = 14;
const int MW_HSLColorspace = 15;
const int MW_HWBColorspace = 16;
const int MW_HammingFilter = 6;
const int MW_HanningFilter = 5;
const int MW_HardLightCompositeOp = 30;
const int MW_HermiteFilter = 4;
const int MW_HueCompositeOp = 31;
const int MW_HuePreview = 4;
const int MW_ImageError = 465;
const int MW_ImageFatalError = 765;
const int MW_ImageWarning = 365;
const int MW_ImplodePreview = 25;
const int MW_ImpulseNoise = 4;
const int MW_InCompositeOp = 32;
const int MW_IndexChannel = 32;
const int MW_IntegerPixel = 4;
const int MW_ItalicStyle = 2;
const int MW_JPEGCompression = 5;
const int MW_JPEGPreview = 29;
const int MW_LABColorspace = 5;
const int MW_LZWCompression = 8;
const int MW_LanczosFilter = 13;
const int MW_LaplacianNoise = 5;
const int MW_LeftAlign = 1;
const int MW_LeftShiftEvaluateOperator = 4;
const int MW_LightenCompositeOp = 33;
const int MW_LineInterlace = 2;
const int MW_LineThroughDecoration = 4;
const int MW_LongPixel = 5;
const int MW_LosslessJPEGCompression = 7;
const int MW_LuminizeCompositeOp = 35;
const int MW_MagentaChannel = 2;
const int MW_MapResource = 4;
const int MW_MaxEvaluateOperator = 5;
const int MW_MaxRGB = 65535;
const int MW_MeanAbsoluteErrorMetric = 2;
const int MW_MeanSquaredErrorMetric = 4;
const int MW_MemoryResource = 5;
const int MW_MinEvaluateOperator = 6;
const int MW_MinusCompositeOp = 36;
const int MW_MirrorVirtualPixelMethod = 5;
const int MW_MissingDelegateError = 420;
const int MW_MissingDelegateFatalError = 720;
const int MW_MissingDelegateWarning = 320;
const int MW_MitchellFilter = 12;
const int MW_MiterJoin = 1;
const int MW_ModulateCompositeOp = 37;
const int MW_ModuleError = 455;
const int MW_ModuleFatalError = 755;
const int MW_ModuleWarning = 355;
const int MW_MonitorError = 485;
const int MW_MonitorFatalError = 785;
const int MW_MonitorWarning = 385;
const int MW_MultiplicativeGaussianNoise = 3;
const int MW_MultiplyCompositeOp = 38;
const int MW_MultiplyEvaluateOperator = 7;
const int MW_NoCompositeOp = 1;
const int MW_NoCompression = 1;
const int MW_NoDecoration = 1;
const int MW_NoInterlace = 1;
const int MW_NonZeroRule = 2;
const int MW_NoneDispose = 1;
const int MW_NormalStretch = 1;
const int MW_NormalStyle = 1;
const int MW_NorthEastGravity = 3;
const int MW_NorthGravity = 2;
const int MW_NorthWestGravity = 1;
const int MW_OHTAColorspace = 4;
const int MW_ObjectBoundingBox = 3;
const int MW_ObliqueStyle = 3;
const int MW_OilPaintPreview = 27;
const int MW_OpacityChannel = 8;
const int MW_OpaqueOpacity = 0;
const int MW_OptimizeType = 10;
const int MW_OptionError = 410;
const int MW_OptionFatalError = 710;
const int MW_OptionWarning = 310;
const int MW_OrEvaluateOperator = 8;
const int MW_OutCompositeOp = 39;
const int MW_OverCompositeOp = 40;
const int MW_OverlayCompositeOp = 41;
const int MW_OverlineDecoration = 3;
const int MW_PaletteMatteType = 5;
const int MW_PaletteType = 4;
const int MW_PartitionInterlace = 4;
const int MW_PeakAbsoluteErrorMetric = 5;
const int MW_PeakSignalToNoiseRatioMetric = 6;
const int MW_PerceptualIntent = 2;
const int MW_PixelsPerCentimeterResolution = 2;
const int MW_PixelsPerInchResolution = 1;
const int MW_PlaneInterlace = 3;
const int MW_PlusCompositeOp = 42;
const int MW_PointFilter = 1;
const int MW_PointMethod = 1;
const int MW_PoissonNoise = 6;
const int MW_PreviousDispose = 3;
const int MW_QuadraticFilter = 9;
const int MW_QuantizePreview = 11;
const int MW_QuantumRange = 65535;
const int MW_RGBColorspace = 1;
const int MW_RLECompression = 9;
const int MW_RaisePreview = 22;
const int MW_RedChannel = 1;
const int MW_ReduceNoisePreview = 13;
const int MW_RegistryError = 490;
const int MW_RegistryFatalError = 790;
const int MW_RegistryWarning = 390;
const int MW_RelativeIntent = 4;
const int MW_ReplaceCompositeOp = 43;
const int MW_ReplaceMethod = 2;
const int MW_ResetMethod = 5;
const int MW_ResourceLimitError = 400;
const int MW_ResourceLimitFatalError = 700;
const int MW_ResourceLimitWarning = 300;
const int MW_RightAlign = 3;
const int MW_RightShiftEvaluateOperator = 9;
const int MW_RollPreview = 3;
const int MW_RootMeanSquaredErrorMetric = 7;
const int MW_RotatePreview = 1;
const int MW_RoundCap = 2;
const int MW_RoundJoin = 2;
const int MW_SaturateCompositeOp = 44;
const int MW_SaturationIntent = 1;
const int MW_SaturationPreview = 5;
const int MW_ScreenCompositeOp = 45;
const int MW_SegmentPreview = 23;
const int MW_SemiCondensedStretch = 5;
const int MW_SemiExpandedStretch = 6;
const int MW_SetEvaluateOperator = 10;
const int MW_ShadePreview = 21;
const int MW_SharpenPreview = 15;
const int MW_ShearPreview = 2;
const int MW_ShortPixel = 7;
const int MW_SincFilter = 15;
const int MW_SoftLightCompositeOp = 46;
const int MW_SolarizePreview = 20;
const int MW_SouthEastGravity = 9;
const int MW_SouthGravity = 8;
const int MW_SouthWestGravity = 7;
const int MW_SpiffPreview = 8;
const int MW_SpreadPreview = 19;
const int MW_SquareCap = 3;
const int MW_SrcAtopCompositeOp = 47;
const int MW_SrcCompositeOp = 48;
const int MW_SrcInCompositeOp = 49;
const int MW_SrcOutCompositeOp = 50;
const int MW_SrcOverCompositeOp = 51;
const int MW_StaticGravity = 10;
const int MW_StreamError = 440;
const int MW_StreamFatalError = 740;
const int MW_StreamWarning = 340;
const int MW_SubtractCompositeOp = 52;
const int MW_SubtractEvaluateOperator = 11;
const int MW_SwirlPreview = 24;
const int MW_ThresholdCompositeOp = 53;
const int MW_ThresholdPreview = 17;
const int MW_TileVirtualPixelMethod = 7;
const int MW_TransparentColorspace = 3;
const int MW_TransparentOpacity = 65535;
const int MW_TriangleFilter = 3;
const int MW_TrueColorMatteType = 7;
const int MW_TrueColorType = 6;
const int MW_TypeError = 405;
const int MW_TypeFatalError = 705;
const int MW_TypeWarning = 305;
const int MW_UltraCondensedStretch = 2;
const int MW_UltraExpandedStretch = 9;
const int MW_UndefinedAlign = 0;
const int MW_UndefinedCap = 0;
const int MW_UndefinedChannel = 0;
const int MW_UndefinedColorspace = 0;
const int MW_UndefinedCompositeOp = 0;
const int MW_UndefinedCompression = 0;
const int MW_UndefinedDecoration = 0;
const int MW_UndefinedDispose = 0;
const int MW_UndefinedEvaluateOperator = 0;
const int MW_UndefinedException = 0;
const int MW_UndefinedFilter = 0;
const int MW_UndefinedGravity = 0;
const int MW_UndefinedIntent = 0;
const int MW_UndefinedInterlace = 0;
const int MW_UndefinedJoin = 0;
const int MW_UndefinedMethod = 0;
const int MW_UndefinedMetric = 0;
const int MW_UndefinedMode = 0;
const int MW_UndefinedNoise = 0;
const int MW_UndefinedPathUnits = 0;
const int MW_UndefinedPixel = 0;
const int MW_UndefinedPreview = 0;
const int MW_UndefinedResolution = 0;
const int MW_UndefinedResource = 0;
const int MW_UndefinedRule = 0;
const int MW_UndefinedStretch = 0;
const int MW_UndefinedStyle = 0;
const int MW_UndefinedType = 0;
const int MW_UndefinedVirtualPixelMethod = 0;
const int MW_UnderlineDecoration = 2;
const int MW_UnframeMode = 2;
const int MW_UniformNoise = 1;
const int MW_UnrecognizedDispose = 0;
const int MW_UserSpace = 1;
const int MW_UserSpaceOnUse = 2;
const int MW_WandError = 470;
const int MW_WandFatalError = 770;
const int MW_WandWarning = 370;
const int MW_WarningException = 300;
const int MW_WavePreview = 26;
const int MW_WestGravity = 4;
const int MW_XYZColorspace = 6;
const int MW_XorCompositeOp = 54;
const int MW_XorEvaluateOperator = 12;
const int MW_YCCColorspace = 8;
const int MW_YCbCrColorspace = 7;
const int MW_YIQColorspace = 9;
const int MW_YPbPrColorspace = 10;
const int MW_YUVColorspace = 11;
const int MW_YellowChannel = 4;
const int MW_ZipCompression = 10;
const int MW_sRGBColorspace = 13;

<<__PHPStdLib>>
function magickgetcopyright();
<<__PHPStdLib>>
function magickgethomeurl();
<<__PHPStdLib>>
function magickgetpackagename();
<<__PHPStdLib>>
function magickgetquantumdepth();
<<__PHPStdLib>>
function magickgetreleasedate();
<<__PHPStdLib>>
function magickgetresourcelimit($resource_type);
<<__PHPStdLib>>
function magickgetversion();
<<__PHPStdLib>>
function magickgetversionnumber();
<<__PHPStdLib>>
function magickgetversionstring();
<<__PHPStdLib>>
function magickqueryconfigureoption($option);
<<__PHPStdLib>>
function magickqueryconfigureoptions($pattern);
<<__PHPStdLib>>
function magickqueryfonts($pattern);
<<__PHPStdLib>>
function magickqueryformats($pattern);
<<__PHPStdLib>>
function magicksetresourcelimit($resource_type, $limit);
<<__PHPStdLib>>
function newdrawingwand();
<<__PHPStdLib>>
function newmagickwand();
<<__PHPStdLib>>
function newpixeliterator($mgck_wnd);
<<__PHPStdLib>>
function newpixelregioniterator($mgck_wnd, $x, $y, $columns, $rows);
<<__PHPStdLib>>
function newpixelwand($imagemagick_col_str = null);
<<__PHPStdLib>>
function newpixelwandarray($num_pxl_wnds);
<<__PHPStdLib>>
function newpixelwands($num_pxl_wnds);
<<__PHPStdLib>>
function destroydrawingwand($drw_wnd);
<<__PHPStdLib>>
function destroymagickwand($mgck_wnd);
<<__PHPStdLib>>
function destroypixeliterator($pxl_iter);
<<__PHPStdLib>>
function destroypixelwand($pxl_wnd);
<<__PHPStdLib>>
function destroypixelwandarray($pxl_wnd_array);
<<__PHPStdLib>>
function destroypixelwands($pxl_wnd_array);
<<__PHPStdLib>>
function isdrawingwand($var);
<<__PHPStdLib>>
function ismagickwand($var);
<<__PHPStdLib>>
function ispixeliterator($var);
<<__PHPStdLib>>
function ispixelwand($var);
<<__PHPStdLib>>
function cleardrawingwand($drw_wnd);
<<__PHPStdLib>>
function clearmagickwand($mgck_wnd);
<<__PHPStdLib>>
function clearpixeliterator($pxl_iter);
<<__PHPStdLib>>
function clearpixelwand($pxl_wnd);
<<__PHPStdLib>>
function clonedrawingwand($drw_wnd);
<<__PHPStdLib>>
function clonemagickwand($mgck_wnd);
<<__PHPStdLib>>
function wandgetexception($wnd);
<<__PHPStdLib>>
function wandgetexceptionstring($wnd);
<<__PHPStdLib>>
function wandgetexceptiontype($wnd);
<<__PHPStdLib>>
function wandhasexception($wnd);
<<__PHPStdLib>>
function drawaffine($drw_wnd, $sx, $sy, $rx, $ry, $tx, $ty);
<<__PHPStdLib>>
function drawannotation($drw_wnd, $x, $y, $text);
<<__PHPStdLib>>
function drawarc($drw_wnd, $sx, $sy, $ex, $ey, $sd, $ed);
<<__PHPStdLib>>
function drawbezier($drw_wnd, $x_y_points_array);
<<__PHPStdLib>>
function drawcircle($drw_wnd, $ox, $oy, $px, $py);
<<__PHPStdLib>>
function drawcolor($drw_wnd, $x, $y, $paint_method);
<<__PHPStdLib>>
function drawcomment($drw_wnd, $comment);
<<__PHPStdLib>>
function drawcomposite($drw_wnd, $composite_operator, $x, $y, $width, $height, $mgck_wnd);
<<__PHPStdLib>>
function drawellipse($drw_wnd, $ox, $oy, $rx, $ry, $start, $end);
<<__PHPStdLib>>
function drawgetclippath($drw_wnd);
<<__PHPStdLib>>
function drawgetcliprule($drw_wnd);
<<__PHPStdLib>>
function drawgetclipunits($drw_wnd);
<<__PHPStdLib>>
function drawgetexception($drw_wnd);
<<__PHPStdLib>>
function drawgetexceptionstring($drw_wnd);
<<__PHPStdLib>>
function drawgetexceptiontype($drw_wnd);
<<__PHPStdLib>>
function drawgetfillalpha($drw_wnd);
<<__PHPStdLib>>
function drawgetfillcolor($drw_wnd);
<<__PHPStdLib>>
function drawgetfillopacity($drw_wnd);
<<__PHPStdLib>>
function drawgetfillrule($drw_wnd);
<<__PHPStdLib>>
function drawgetfont($drw_wnd);
<<__PHPStdLib>>
function drawgetfontfamily($drw_wnd);
<<__PHPStdLib>>
function drawgetfontsize($drw_wnd);
<<__PHPStdLib>>
function drawgetfontstretch($drw_wnd);
<<__PHPStdLib>>
function drawgetfontstyle($drw_wnd);
<<__PHPStdLib>>
function drawgetfontweight($drw_wnd);
<<__PHPStdLib>>
function drawgetgravity($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokealpha($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokeantialias($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokecolor($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokedasharray($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokedashoffset($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokelinecap($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokelinejoin($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokemiterlimit($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokeopacity($drw_wnd);
<<__PHPStdLib>>
function drawgetstrokewidth($drw_wnd);
<<__PHPStdLib>>
function drawgettextalignment($drw_wnd);
<<__PHPStdLib>>
function drawgettextantialias($drw_wnd);
<<__PHPStdLib>>
function drawgettextdecoration($drw_wnd);
<<__PHPStdLib>>
function drawgettextencoding($drw_wnd);
<<__PHPStdLib>>
function drawgettextundercolor($drw_wnd);
<<__PHPStdLib>>
function drawgetvectorgraphics($drw_wnd);
<<__PHPStdLib>>
function drawline($drw_wnd, $sx, $sy, $ex, $ey);
<<__PHPStdLib>>
function drawmatte($drw_wnd, $x, $y, $paint_method);
<<__PHPStdLib>>
function drawpathclose($drw_wnd);
<<__PHPStdLib>>
function drawpathcurvetoabsolute($drw_wnd, $x1, $y1, $x2, $y2, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierabsolute($drw_wnd, $x1, $y1, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetoquadraticbezierrelative($drw_wnd, $x1, $y1, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothabsolute($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetoquadraticbeziersmoothrelative($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetorelative($drw_wnd, $x1, $y1, $x2, $y2, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetosmoothabsolute($drw_wnd, $x2, $y2, $x, $y);
<<__PHPStdLib>>
function drawpathcurvetosmoothrelative($drw_wnd, $x2, $y2, $x, $y);
<<__PHPStdLib>>
function drawpathellipticarcabsolute($drw_wnd, $rx, $ry, $x_axis_rotation, $large_arc_flag, $sweep_flag, $x, $y);
<<__PHPStdLib>>
function drawpathellipticarcrelative($drw_wnd, $rx, $ry, $x_axis_rotation, $large_arc_flag, $sweep_flag, $x, $y);
<<__PHPStdLib>>
function drawpathfinish($drw_wnd);
<<__PHPStdLib>>
function drawpathlinetoabsolute($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathlinetohorizontalabsolute($drw_wnd, $x);
<<__PHPStdLib>>
function drawpathlinetohorizontalrelative($drw_wnd, $x);
<<__PHPStdLib>>
function drawpathlinetorelative($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathlinetoverticalabsolute($drw_wnd, $y);
<<__PHPStdLib>>
function drawpathlinetoverticalrelative($drw_wnd, $y);
<<__PHPStdLib>>
function drawpathmovetoabsolute($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathmovetorelative($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpathstart($drw_wnd);
<<__PHPStdLib>>
function drawpoint($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawpolygon($drw_wnd, $x_y_points_array);
<<__PHPStdLib>>
function drawpolyline($drw_wnd, $x_y_points_array);
<<__PHPStdLib>>
function drawrectangle($drw_wnd, $x1, $y1, $x2, $y2);
<<__PHPStdLib>>
function drawrender($drw_wnd);
<<__PHPStdLib>>
function drawrotate($drw_wnd, $degrees);
<<__PHPStdLib>>
function drawroundrectangle($drw_wnd, $x1, $y1, $x2, $y2, $rx, $ry);
<<__PHPStdLib>>
function drawscale($drw_wnd, $x, $y);
<<__PHPStdLib>>
function drawsetclippath($drw_wnd, $clip_path);
<<__PHPStdLib>>
function drawsetcliprule($drw_wnd, $fill_rule);
<<__PHPStdLib>>
function drawsetclipunits($drw_wnd, $clip_path_units);
<<__PHPStdLib>>
function drawsetfillalpha($drw_wnd, $fill_opacity);
<<__PHPStdLib>>
function drawsetfillcolor($drw_wnd, $fill_pxl_wnd);
<<__PHPStdLib>>
function drawsetfillopacity($drw_wnd, $fill_opacity);
<<__PHPStdLib>>
function drawsetfillpatternurl($drw_wnd, $fill_url);
<<__PHPStdLib>>
function drawsetfillrule($drw_wnd, $fill_rule);
<<__PHPStdLib>>
function drawsetfont($drw_wnd, $font_file);
<<__PHPStdLib>>
function drawsetfontfamily($drw_wnd, $font_family);
<<__PHPStdLib>>
function drawsetfontsize($drw_wnd, $pointsize);
<<__PHPStdLib>>
function drawsetfontstretch($drw_wnd, $stretch_type);
<<__PHPStdLib>>
function drawsetfontstyle($drw_wnd, $style_type);
<<__PHPStdLib>>
function drawsetfontweight($drw_wnd, $font_weight);
<<__PHPStdLib>>
function drawsetgravity($drw_wnd, $gravity_type);
<<__PHPStdLib>>
function drawsetstrokealpha($drw_wnd, $stroke_opacity);
<<__PHPStdLib>>
function drawsetstrokeantialias($drw_wnd, $stroke_antialias = true);
<<__PHPStdLib>>
function drawsetstrokecolor($drw_wnd, $strokecolor_pxl_wnd);
<<__PHPStdLib>>
function drawsetstrokedasharray($drw_wnd, $dash_array = null);
<<__PHPStdLib>>
function drawsetstrokedashoffset($drw_wnd, $dash_offset);
<<__PHPStdLib>>
function drawsetstrokelinecap($drw_wnd, $line_cap);
<<__PHPStdLib>>
function drawsetstrokelinejoin($drw_wnd, $line_join);
<<__PHPStdLib>>
function drawsetstrokemiterlimit($drw_wnd, $miterlimit);
<<__PHPStdLib>>
function drawsetstrokeopacity($drw_wnd, $stroke_opacity);
<<__PHPStdLib>>
function drawsetstrokepatternurl($drw_wnd, $stroke_url);
<<__PHPStdLib>>
function drawsetstrokewidth($drw_wnd, $stroke_width);
<<__PHPStdLib>>
function drawsettextalignment($drw_wnd, $align_type);
<<__PHPStdLib>>
function drawsettextantialias($drw_wnd, $text_antialias = true);
<<__PHPStdLib>>
function drawsettextdecoration($drw_wnd, $decoration_type);
<<__PHPStdLib>>
function drawsettextencoding($drw_wnd, $encoding);
<<__PHPStdLib>>
function drawsettextundercolor($drw_wnd, $undercolor_pxl_wnd);
<<__PHPStdLib>>
function drawsetvectorgraphics($drw_wnd, $vector_graphics);
<<__PHPStdLib>>
function drawsetviewbox($drw_wnd, $x1, $y1, $x2, $y2);
<<__PHPStdLib>>
function drawskewx($drw_wnd, $degrees);
<<__PHPStdLib>>
function drawskewy($drw_wnd, $degrees);
<<__PHPStdLib>>
function drawtranslate($drw_wnd, $x, $y);
<<__PHPStdLib>>
function pushdrawingwand($drw_wnd);
<<__PHPStdLib>>
function drawpushclippath($drw_wnd, $clip_path_id);
<<__PHPStdLib>>
function drawpushdefs($drw_wnd);
<<__PHPStdLib>>
function drawpushpattern($drw_wnd, $pattern_id, $x, $y, $width, $height);
<<__PHPStdLib>>
function popdrawingwand($drw_wnd);
<<__PHPStdLib>>
function drawpopclippath($drw_wnd);
<<__PHPStdLib>>
function drawpopdefs($drw_wnd);
<<__PHPStdLib>>
function drawpoppattern($drw_wnd);
<<__PHPStdLib>>
function magickadaptivethresholdimage($mgck_wnd, $width, $height, $offset);
<<__PHPStdLib>>
function magickaddimage($mgck_wnd, $add_wand);
<<__PHPStdLib>>
function magickaddnoiseimage($mgck_wnd, $noise_type);
<<__PHPStdLib>>
function magickaffinetransformimage($mgck_wnd, $drw_wnd);
<<__PHPStdLib>>
function magickannotateimage($mgck_wnd, $drw_wnd, $x, $y, $angle, $text);
<<__PHPStdLib>>
function magickappendimages($mgck_wnd, $stack_vertical = false);
<<__PHPStdLib>>
function magickaverageimages($mgck_wnd);
<<__PHPStdLib>>
function magickblackthresholdimage($mgck_wnd, $threshold_pxl_wnd);
<<__PHPStdLib>>
function magickblurimage($mgck_wnd, $radius, $sigma, $channel_type = 0);
<<__PHPStdLib>>
function magickborderimage($mgck_wnd, $bordercolor, $width, $height);
<<__PHPStdLib>>
function magickcharcoalimage($mgck_wnd, $radius, $sigma);
<<__PHPStdLib>>
function magickchopimage($mgck_wnd, $width, $height, $x, $y);
<<__PHPStdLib>>
function magickclipimage($mgck_wnd);
<<__PHPStdLib>>
function magickclippathimage($mgck_wnd, $pathname, $inside);
<<__PHPStdLib>>
function magickcoalesceimages($mgck_wnd);
<<__PHPStdLib>>
function magickcolorfloodfillimage($mgck_wnd, $fillcolor_pxl_wnd, $fuzz, $bordercolor_pxl_wnd, $x, $y);
<<__PHPStdLib>>
function magickcolorizeimage($mgck_wnd, $colorize, $opacity_pxl_wnd);
<<__PHPStdLib>>
function magickcombineimages($mgck_wnd, $channel_type);
<<__PHPStdLib>>
function magickcommentimage($mgck_wnd, $comment);
<<__PHPStdLib>>
function magickcompareimages($mgck_wnd, $reference_wnd, $metric_type, $channel_type = 0);
<<__PHPStdLib>>
function magickcompositeimage($mgck_wnd, $composite_wnd, $composite_operator, $x, $y);
<<__PHPStdLib>>
function magickconstituteimage($mgck_wnd, $columns, $rows, $smap, $storage_type, $pixel_array);
<<__PHPStdLib>>
function magickcontrastimage($mgck_wnd, $sharpen);
<<__PHPStdLib>>
function magickconvolveimage($mgck_wnd, $kernel_array, $channel_type = 0);
<<__PHPStdLib>>
function magickcropimage($mgck_wnd, $width, $height, $x, $y);
<<__PHPStdLib>>
function magickcyclecolormapimage($mgck_wnd, $num_positions);
<<__PHPStdLib>>
function magickdeconstructimages($mgck_wnd);
<<__PHPStdLib>>
function magickdescribeimage($mgck_wnd);
<<__PHPStdLib>>
function magickdespeckleimage($mgck_wnd);
<<__PHPStdLib>>
function magickdrawimage($mgck_wnd, $drw_wnd);
<<__PHPStdLib>>
function magickechoimageblob($mgck_wnd);
<<__PHPStdLib>>
function magickechoimagesblob($mgck_wnd);
<<__PHPStdLib>>
function magickedgeimage($mgck_wnd, $radius);
<<__PHPStdLib>>
function magickembossimage($mgck_wnd, $radius, $sigma);
<<__PHPStdLib>>
function magickenhanceimage($mgck_wnd);
<<__PHPStdLib>>
function magickequalizeimage($mgck_wnd);
<<__PHPStdLib>>
function magickevaluateimage($mgck_wnd, $evaluate_op, $constant, $channel_type = 0);
<<__PHPStdLib>>
function magickflattenimages($mgck_wnd);
<<__PHPStdLib>>
function magickflipimage($mgck_wnd);
<<__PHPStdLib>>
function magickflopimage($mgck_wnd);
<<__PHPStdLib>>
function magickframeimage($mgck_wnd, $matte_color, $width, $height, $inner_bevel, $outer_bevel);
<<__PHPStdLib>>
function magickfximage($mgck_wnd, $expression, $channel_type = 0);
<<__PHPStdLib>>
function magickgammaimage($mgck_wnd, $gamma, $channel_type = 0);
<<__PHPStdLib>>
function magickgaussianblurimage($mgck_wnd, $radius, $sigma, $channel_type = 0);
<<__PHPStdLib>>
function magickgetcharheight($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgetcharwidth($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgetexception($mgck_wnd);
<<__PHPStdLib>>
function magickgetexceptionstring($mgck_wnd);
<<__PHPStdLib>>
function magickgetexceptiontype($mgck_wnd);
<<__PHPStdLib>>
function magickgetfilename($mgck_wnd);
<<__PHPStdLib>>
function magickgetformat($mgck_wnd);
<<__PHPStdLib>>
function magickgetimage($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagebackgroundcolor($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageblob($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageblueprimary($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagebordercolor($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagechannelmean($mgck_wnd, $channel_type);
<<__PHPStdLib>>
function magickgetimagecolormapcolor($mgck_wnd, $index);
<<__PHPStdLib>>
function magickgetimagecolors($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagecolorspace($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagecompose($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagecompression($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagecompressionquality($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagedelay($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagedepth($mgck_wnd, $channel_type = 0);
<<__PHPStdLib>>
function magickgetimagedispose($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageextrema($mgck_wnd, $channel_type = 0);
<<__PHPStdLib>>
function magickgetimagefilename($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageformat($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagegamma($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagegreenprimary($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageheight($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagehistogram($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageindex($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageinterlacescheme($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageiterations($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagemattecolor($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagemimetype($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagepixels($mgck_wnd, $x_offset, $y_offset, $columns, $rows, $smap, $storage_type);
<<__PHPStdLib>>
function magickgetimageprofile($mgck_wnd, $name);
<<__PHPStdLib>>
function magickgetimageredprimary($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagerenderingintent($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageresolution($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagescene($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagesignature($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagesize($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagetype($mgck_wnd);
<<__PHPStdLib>>
function magickgetimageunits($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagevirtualpixelmethod($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagewhitepoint($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagewidth($mgck_wnd);
<<__PHPStdLib>>
function magickgetimagesblob($mgck_wnd);
<<__PHPStdLib>>
function magickgetinterlacescheme($mgck_wnd);
<<__PHPStdLib>>
function magickgetmaxtextadvance($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgetmimetype($mgck_wnd);
<<__PHPStdLib>>
function magickgetnumberimages($mgck_wnd);
<<__PHPStdLib>>
function magickgetsamplingfactors($mgck_wnd);
<<__PHPStdLib>>
function magickgetsize($mgck_wnd);
<<__PHPStdLib>>
function magickgetstringheight($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgetstringwidth($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgettextascent($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgettextdescent($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickgetwandsize($mgck_wnd);
<<__PHPStdLib>>
function magickhasnextimage($mgck_wnd);
<<__PHPStdLib>>
function magickhaspreviousimage($mgck_wnd);
<<__PHPStdLib>>
function magickimplodeimage($mgck_wnd, $amount);
<<__PHPStdLib>>
function magicklabelimage($mgck_wnd, $label);
<<__PHPStdLib>>
function magicklevelimage($mgck_wnd, $black_point, $gamma, $white_point, $channel_type = 0);
<<__PHPStdLib>>
function magickmagnifyimage($mgck_wnd);
<<__PHPStdLib>>
function magickmapimage($mgck_wnd, $map_wand, $dither);
<<__PHPStdLib>>
function magickmattefloodfillimage($mgck_wnd, $opacity, $fuzz, $bordercolor_pxl_wnd, $x, $y);
<<__PHPStdLib>>
function magickmedianfilterimage($mgck_wnd, $radius);
<<__PHPStdLib>>
function magickminifyimage($mgck_wnd);
<<__PHPStdLib>>
function magickmodulateimage($mgck_wnd, $brightness, $saturation, $hue);
<<__PHPStdLib>>
function magickmontageimage($mgck_wnd, $drw_wnd, $tile_geometry, $thumbnail_geometry, $montage_mode, $frame);
<<__PHPStdLib>>
function magickmorphimages($mgck_wnd, $number_frames);
<<__PHPStdLib>>
function magickmosaicimages($mgck_wnd);
<<__PHPStdLib>>
function magickmotionblurimage($mgck_wnd, $radius, $sigma, $angle);
<<__PHPStdLib>>
function magicknegateimage($mgck_wnd, $only_the_gray = false, $channel_type = 0);
<<__PHPStdLib>>
function magicknewimage($mgck_wnd, $width, $height, $imagemagick_col_str = null);
<<__PHPStdLib>>
function magicknextimage($mgck_wnd);
<<__PHPStdLib>>
function magicknormalizeimage($mgck_wnd);
<<__PHPStdLib>>
function magickoilpaintimage($mgck_wnd, $radius);
<<__PHPStdLib>>
function magickpaintopaqueimage($mgck_wnd, $target_pxl_wnd, $fill_pxl_wnd, $fuzz = 0.0);
<<__PHPStdLib>>
function magickpainttransparentimage($mgck_wnd, $target, $opacity = MW_TransparentOpacity, $fuzz = 0.0);
<<__PHPStdLib>>
function magickpingimage($mgck_wnd, $filename);
<<__PHPStdLib>>
function magickposterizeimage($mgck_wnd, $levels, $dither);
<<__PHPStdLib>>
function magickpreviewimages($mgck_wnd, $preview);
<<__PHPStdLib>>
function magickpreviousimage($mgck_wnd);
<<__PHPStdLib>>
function magickprofileimage($mgck_wnd, $name, $profile = null);
<<__PHPStdLib>>
function magickquantizeimage($mgck_wnd, $number_colors, $colorspace_type, $treedepth, $dither, $measure_error);
<<__PHPStdLib>>
function magickquantizeimages($mgck_wnd, $number_colors, $colorspace_type, $treedepth, $dither, $measure_error);
<<__PHPStdLib>>
function magickqueryfontmetrics($mgck_wnd, $drw_wnd, $txt, $multiline = false);
<<__PHPStdLib>>
function magickradialblurimage($mgck_wnd, $angle);
<<__PHPStdLib>>
function magickraiseimage($mgck_wnd, $width, $height, $x, $y, $raise);
<<__PHPStdLib>>
function magickreadimage($mgck_wnd, $filename);
<<__PHPStdLib>>
function magickreadimageblob($mgck_wnd, $blob);
<<__PHPStdLib>>
function magickreadimagefile($mgck_wnd, $handle);
<<__PHPStdLib>>
function magickreadimages($mgck_wnd, $img_filenames_array);
<<__PHPStdLib>>
function magickreducenoiseimage($mgck_wnd, $radius);
<<__PHPStdLib>>
function magickremoveimage($mgck_wnd);
<<__PHPStdLib>>
function magickremoveimageprofile($mgck_wnd, $name);
<<__PHPStdLib>>
function magickremoveimageprofiles($mgck_wnd);
<<__PHPStdLib>>
function magickresampleimage($mgck_wnd, $x_resolution, $y_resolution, $filter_type, $blur);
<<__PHPStdLib>>
function magickresetiterator($mgck_wnd);
<<__PHPStdLib>>
function magickresizeimage($mgck_wnd, $columns, $rows, $filter_type, $blur);
<<__PHPStdLib>>
function magickrollimage($mgck_wnd, $x_offset, $y_offset);
<<__PHPStdLib>>
function magickrotateimage($mgck_wnd, $background, $degrees);
<<__PHPStdLib>>
function magicksampleimage($mgck_wnd, $columns, $rows);
<<__PHPStdLib>>
function magickscaleimage($mgck_wnd, $columns, $rows);
<<__PHPStdLib>>
function magickseparateimagechannel($mgck_wnd, $channel_type);
<<__PHPStdLib>>
function magicksetcompressionquality($mgck_wnd, $quality);
<<__PHPStdLib>>
function magicksetfilename($mgck_wnd, $filename = null);
<<__PHPStdLib>>
function magicksetfirstiterator($mgck_wnd);
<<__PHPStdLib>>
function magicksetformat($mgck_wnd, $format);
<<__PHPStdLib>>
function magicksetimage($mgck_wnd, $replace_wand);
<<__PHPStdLib>>
function magicksetimagebackgroundcolor($mgck_wnd, $background_pxl_wnd);
<<__PHPStdLib>>
function magicksetimagebias($mgck_wnd, $bias);
<<__PHPStdLib>>
function magicksetimageblueprimary($mgck_wnd, $x, $y);
<<__PHPStdLib>>
function magicksetimagebordercolor($mgck_wnd, $border_pxl_wnd);
<<__PHPStdLib>>
function magicksetimagecolormapcolor($mgck_wnd, $index, $mapcolor_pxl_wnd);
<<__PHPStdLib>>
function magicksetimagecolorspace($mgck_wnd, $colorspace_type);
<<__PHPStdLib>>
function magicksetimagecompose($mgck_wnd, $composite_operator);
<<__PHPStdLib>>
function magicksetimagecompression($mgck_wnd, $compression_type);
<<__PHPStdLib>>
function magicksetimagecompressionquality($mgck_wnd, $quality);
<<__PHPStdLib>>
function magicksetimagedelay($mgck_wnd, $delay);
<<__PHPStdLib>>
function magicksetimagedepth($mgck_wnd, $depth, $channel_type = 0);
<<__PHPStdLib>>
function magicksetimagedispose($mgck_wnd, $dispose_type);
<<__PHPStdLib>>
function magicksetimagefilename($mgck_wnd, $filename = null);
<<__PHPStdLib>>
function magicksetimageformat($mgck_wnd, $format);
<<__PHPStdLib>>
function magicksetimagegamma($mgck_wnd, $gamma);
<<__PHPStdLib>>
function magicksetimagegreenprimary($mgck_wnd, $x, $y);
<<__PHPStdLib>>
function magicksetimageindex($mgck_wnd, $index);
<<__PHPStdLib>>
function magicksetimageinterlacescheme($mgck_wnd, $interlace_type);
<<__PHPStdLib>>
function magicksetimageiterations($mgck_wnd, $iterations);
<<__PHPStdLib>>
function magicksetimagemattecolor($mgck_wnd, $matte_pxl_wnd);
<<__PHPStdLib>>
function magicksetimageoption($mgck_wnd, $format, $key, $value);
<<__PHPStdLib>>
function magicksetimagepixels($mgck_wnd, $x_offset, $y_offset, $columns, $rows, $smap, $storage_type, $pixel_array);
<<__PHPStdLib>>
function magicksetimageprofile($mgck_wnd, $name, $profile);
<<__PHPStdLib>>
function magicksetimageredprimary($mgck_wnd, $x, $y);
<<__PHPStdLib>>
function magicksetimagerenderingintent($mgck_wnd, $rendering_intent);
<<__PHPStdLib>>
function magicksetimageresolution($mgck_wnd, $x_resolution, $y_resolution);
<<__PHPStdLib>>
function magicksetimagescene($mgck_wnd, $scene);
<<__PHPStdLib>>
function magicksetimagetype($mgck_wnd, $image_type);
<<__PHPStdLib>>
function magicksetimageunits($mgck_wnd, $resolution_type);
<<__PHPStdLib>>
function magicksetimagevirtualpixelmethod($mgck_wnd, $virtual_pixel_method);
<<__PHPStdLib>>
function magicksetimagewhitepoint($mgck_wnd, $x, $y);
<<__PHPStdLib>>
function magicksetinterlacescheme($mgck_wnd, $interlace_type);
<<__PHPStdLib>>
function magicksetlastiterator($mgck_wnd);
<<__PHPStdLib>>
function magicksetpassphrase($mgck_wnd, $passphrase);
<<__PHPStdLib>>
function magicksetresolution($mgck_wnd, $x_resolution, $y_resolution);
<<__PHPStdLib>>
function magicksetsamplingfactors($mgck_wnd, $number_factors, $sampling_factors);
<<__PHPStdLib>>
function magicksetsize($mgck_wnd, $columns, $rows);
<<__PHPStdLib>>
function magicksetwandsize($mgck_wnd, $columns, $rows);
<<__PHPStdLib>>
function magicksharpenimage($mgck_wnd, $radius, $sigma, $channel_type = 0);
<<__PHPStdLib>>
function magickshaveimage($mgck_wnd, $columns, $rows);
<<__PHPStdLib>>
function magickshearimage($mgck_wnd, $background, $x_shear, $y_shear);
<<__PHPStdLib>>
function magicksolarizeimage($mgck_wnd, $threshold);
<<__PHPStdLib>>
function magickspliceimage($mgck_wnd, $width, $height, $x, $y);
<<__PHPStdLib>>
function magickspreadimage($mgck_wnd, $radius);
<<__PHPStdLib>>
function magicksteganoimage($mgck_wnd, $watermark_wand, $offset);
<<__PHPStdLib>>
function magickstereoimage($mgck_wnd, $offset_wand);
<<__PHPStdLib>>
function magickstripimage($mgck_wnd);
<<__PHPStdLib>>
function magickswirlimage($mgck_wnd, $degrees);
<<__PHPStdLib>>
function magicktextureimage($mgck_wnd, $texture_wand);
<<__PHPStdLib>>
function magickthresholdimage($mgck_wnd, $threshold, $channel_type = 0);
<<__PHPStdLib>>
function magicktintimage($mgck_wnd, $tint_pxl_wnd, $opacity_pxl_wnd);
<<__PHPStdLib>>
function magicktransformimage($mgck_wnd, $crop, $geometry);
<<__PHPStdLib>>
function magicktrimimage($mgck_wnd, $fuzz);
<<__PHPStdLib>>
function magickunsharpmaskimage($mgck_wnd, $radius, $sigma, $amount, $threshold, $channel_type = 0);
<<__PHPStdLib>>
function magickwaveimage($mgck_wnd, $amplitude, $wave_length);
<<__PHPStdLib>>
function magickwhitethresholdimage($mgck_wnd, $threshold_pxl_wnd);
<<__PHPStdLib>>
function magickwriteimage($mgck_wnd, $filename);
<<__PHPStdLib>>
function magickwriteimagefile($mgck_wnd, $handle);
<<__PHPStdLib>>
function magickwriteimages($mgck_wnd, $filename = "", $join_images = false);
<<__PHPStdLib>>
function magickwriteimagesfile($mgck_wnd, $handle);
<<__PHPStdLib>>
function pixelgetalpha($pxl_wnd);
<<__PHPStdLib>>
function pixelgetalphaquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetblack($pxl_wnd);
<<__PHPStdLib>>
function pixelgetblackquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetblue($pxl_wnd);
<<__PHPStdLib>>
function pixelgetbluequantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetcolorasstring($pxl_wnd);
<<__PHPStdLib>>
function pixelgetcolorcount($pxl_wnd);
<<__PHPStdLib>>
function pixelgetcyan($pxl_wnd);
<<__PHPStdLib>>
function pixelgetcyanquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetexception($pxl_wnd);
<<__PHPStdLib>>
function pixelgetexceptionstring($pxl_wnd);
<<__PHPStdLib>>
function pixelgetexceptiontype($pxl_wnd);
<<__PHPStdLib>>
function pixelgetgreen($pxl_wnd);
<<__PHPStdLib>>
function pixelgetgreenquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetindex($pxl_wnd);
<<__PHPStdLib>>
function pixelgetmagenta($pxl_wnd);
<<__PHPStdLib>>
function pixelgetmagentaquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetopacity($pxl_wnd);
<<__PHPStdLib>>
function pixelgetopacityquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetquantumcolor($pxl_wnd);
<<__PHPStdLib>>
function pixelgetred($pxl_wnd);
<<__PHPStdLib>>
function pixelgetredquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelgetyellow($pxl_wnd);
<<__PHPStdLib>>
function pixelgetyellowquantum($pxl_wnd);
<<__PHPStdLib>>
function pixelsetalpha($pxl_wnd, $alpha);
<<__PHPStdLib>>
function pixelsetalphaquantum($pxl_wnd, $alpha);
<<__PHPStdLib>>
function pixelsetblack($pxl_wnd, $black);
<<__PHPStdLib>>
function pixelsetblackquantum($pxl_wnd, $black);
<<__PHPStdLib>>
function pixelsetblue($pxl_wnd, $blue);
<<__PHPStdLib>>
function pixelsetbluequantum($pxl_wnd, $blue);
<<__PHPStdLib>>
function pixelsetcolor($pxl_wnd, $imagemagick_col_str);
<<__PHPStdLib>>
function pixelsetcolorcount($pxl_wnd, $count);
<<__PHPStdLib>>
function pixelsetcyan($pxl_wnd, $cyan);
<<__PHPStdLib>>
function pixelsetcyanquantum($pxl_wnd, $cyan);
<<__PHPStdLib>>
function pixelsetgreen($pxl_wnd, $green);
<<__PHPStdLib>>
function pixelsetgreenquantum($pxl_wnd, $green);
<<__PHPStdLib>>
function pixelsetindex($pxl_wnd, $index);
<<__PHPStdLib>>
function pixelsetmagenta($pxl_wnd, $magenta);
<<__PHPStdLib>>
function pixelsetmagentaquantum($pxl_wnd, $magenta);
<<__PHPStdLib>>
function pixelsetopacity($pxl_wnd, $opacity);
<<__PHPStdLib>>
function pixelsetopacityquantum($pxl_wnd, $opacity);
<<__PHPStdLib>>
function pixelsetquantumcolor($pxl_wnd, $red, $green, $blue, $opacity = 0.0);
<<__PHPStdLib>>
function pixelsetred($pxl_wnd, $red);
<<__PHPStdLib>>
function pixelsetredquantum($pxl_wnd, $red);
<<__PHPStdLib>>
function pixelsetyellow($pxl_wnd, $yellow);
<<__PHPStdLib>>
function pixelsetyellowquantum($pxl_wnd, $yellow);
<<__PHPStdLib>>
function pixelgetiteratorexception($pxl_iter);
<<__PHPStdLib>>
function pixelgetiteratorexceptionstring($pxl_iter);
<<__PHPStdLib>>
function pixelgetiteratorexceptiontype($pxl_iter);
<<__PHPStdLib>>
function pixelgetnextiteratorrow($pxl_iter);
<<__PHPStdLib>>
function pixelresetiterator($pxl_iter);
<<__PHPStdLib>>
function pixelsetiteratorrow($pxl_iter, $row);
<<__PHPStdLib>>
function pixelsynciterator($pxl_iter);
