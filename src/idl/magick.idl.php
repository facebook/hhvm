<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// Resources

f('MagickGetCopyright',          String);
f('MagickGetHomeURL',            String);
f('MagickGetPackageName',        String);
f('MagickGetQuantumDepth',       Double);
f('MagickGetReleaseDate',        String);
f('MagickGetResourceLimit',      Double,     array('resource_type' => Int32));
f('MagickGetVersion',            VariantVec);
f('MagickGetVersionNumber',      Int32);
f('MagickGetVersionString',      String);
f('MagickQueryConfigureOption',  String,     array('option' => String));
f('MagickQueryConfigureOptions', StringVec,  array('pattern' => String));
f('MagickQueryFonts',            VariantVec, array('pattern' => String));
f('MagickQueryFormats',          VariantVec, array('pattern' => String));

f('MagickSetResourceLimit', Boolean,
  array('resource_type' => Int32,
        'limit' => Double));

f('NewDrawingWand',         Resource);
f('NewMagickWand',          Resource);
f('NewPixelIterator',       Resource,  array('mgck_wnd' => Resource));
f('NewPixelRegionIterator', Resource,
  array('mgck_wnd' => Resource,
        'x' => Int32,
        'y' => Int32,
        'columns' => Int32,
        'rows' => Int32));
f('NewPixelWand',           Resource,
  array('imagemagick_col_str' => array(String, 'null_string')));
f('NewPixelWandArray',      VariantVec,  array('num_pxl_wnds' => Int32));
f('NewPixelWands',          VariantVec,  array('num_pxl_wnds' => Int32));

f('DestroyDrawingWand',     NULL,        array('drw_wnd' => Resource));
f('DestroyMagickWand',      NULL,        array('mgck_wnd' => Resource));
f('DestroyPixelIterator',   NULL,        array('pxl_iter' => Resource));
f('DestroyPixelWand',       NULL,        array('pxl_wnd' => Resource));
f('DestroyPixelWandArray',  NULL,        array('pxl_wnd_array' => VariantVec));
f('DestroyPixelWands',      NULL,        array('pxl_wnd_array' => VariantVec));

f('IsDrawingWand',          Boolean,     array('var' => Variant));
f('IsMagickWand',           Boolean,     array('var' => Variant));
f('IsPixelIterator',        Boolean,     array('var' => Variant));
f('IsPixelWand',            Boolean,     array('var' => Variant));

f('ClearDrawingWand',       NULL,        array('drw_wnd' => Resource));
f('ClearMagickWand',        NULL,        array('mgck_wnd' => Resource));
f('ClearPixelIterator',     NULL,        array('pxl_iter' => Resource));
f('ClearPixelWand',         NULL,        array('pxl_wnd' => Resource));

f('CloneDrawingWand',       Resource,    array('drw_wnd' => Resource));
f('CloneMagickWand',        Resource,    array('mgck_wnd' => Resource));

f('WandGetException',       VariantVec,  array('wnd' => Resource));
f('WandGetExceptionString', String,      array('wnd' => Resource));
f('WandGetExceptionType',   Int32,       array('wnd' => Resource));
f('WandHasException',       Boolean,     array('wnd' => Resource));

///////////////////////////////////////////////////////////////////////////////
// Draw Functions

f('DrawAffine', NULL,
  array('drw_wnd' => Resource,
        'sx' => Double,
        'sy' => Double,
        'rx' => Double,
        'ry' => Double,
        'tx' => Double,
        'ty' => Double));

f('DrawAnnotation', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double,
        'text' => String));

f('DrawArc', NULL,
  array('drw_wnd' => Resource,
        'sx' => Double,
        'sy' => Double,
        'ex' => Double,
        'ey' => Double,
        'sd' => Double,
        'ed' => Double));

f('DrawBezier', NULL,
  array('drw_wnd' => Resource,
        'x_y_points_array' => VariantVec));

f('DrawCircle', NULL,
  array('drw_wnd' => Resource,
        'ox' => Double,
        'oy' => Double,
        'px' => Double,
        'py' => Double));

f('DrawColor', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double,
        'paint_method' => Int32));

f('DrawComment', NULL,
  array('drw_wnd' => Resource,
        'comment' => String));

f('DrawComposite', Boolean,
  array('drw_wnd' => Resource,
        'composite_operator' => Int32,
        'x' => Double,
        'y' => Double,
        'width' => Double,
        'height' => Double,
        'mgck_wnd' => Resource));

f('DrawEllipse', NULL,
  array('drw_wnd' => Resource,
        'ox' => Double,
        'oy' => Double,
        'rx' => Double,
        'ry' => Double,
        'start' => Double,
        'end' => Double));

f('DrawGetClipPath',         String,     array('drw_wnd' => Resource));
f('DrawGetClipRule',         Int32,      array('drw_wnd' => Resource));
f('DrawGetClipUnits',        Int32,      array('drw_wnd' => Resource));
f('DrawGetException',        VariantVec, array('drw_wnd' => Resource));
f('DrawGetExceptionString',  String,     array('drw_wnd' => Resource));
f('DrawGetExceptionType',    Int32,      array('drw_wnd' => Resource));
f('DrawGetFillAlpha',        Double,     array('drw_wnd' => Resource));
f('DrawGetFillColor',        Resource,   array('drw_wnd' => Resource));
f('DrawGetFillOpacity',      Double,     array('drw_wnd' => Resource));
f('DrawGetFillRule',         Int32,      array('drw_wnd' => Resource));
f('DrawGetFont',             String,     array('drw_wnd' => Resource));
f('DrawGetFontFamily',       String,     array('drw_wnd' => Resource));
f('DrawGetFontSize',         Double,     array('drw_wnd' => Resource));
f('DrawGetFontStretch',      Int32,      array('drw_wnd' => Resource));
f('DrawGetFontStyle',        Int32,      array('drw_wnd' => Resource));
f('DrawGetFontWeight',       Double,     array('drw_wnd' => Resource));
f('DrawGetGravity',          Int32,      array('drw_wnd' => Resource));
f('DrawGetStrokeAlpha',      Double,     array('drw_wnd' => Resource));
f('DrawGetStrokeAntialias',  Boolean,    array('drw_wnd' => Resource));
f('DrawGetStrokeColor',      Resource,   array('drw_wnd' => Resource));
f('DrawGetStrokeDashArray',  VariantVec, array('drw_wnd' => Resource));
f('DrawGetStrokeDashOffset', Double,     array('drw_wnd' => Resource));
f('DrawGetStrokeLineCap',    Int32,      array('drw_wnd' => Resource));
f('DrawGetStrokeLineJoin',   Int32,      array('drw_wnd' => Resource));
f('DrawGetStrokeMiterLimit', Double,     array('drw_wnd' => Resource));
f('DrawGetStrokeOpacity',    Double,     array('drw_wnd' => Resource));
f('DrawGetStrokeWidth',      Double,     array('drw_wnd' => Resource));
f('DrawGetTextAlignment',    Int32,      array('drw_wnd' => Resource));
f('DrawGetTextAntialias',    Boolean,    array('drw_wnd' => Resource));
f('DrawGetTextDecoration',   Int32,      array('drw_wnd' => Resource));
f('DrawGetTextEncoding',     String,     array('drw_wnd' => Resource));
f('DrawGetTextUnderColor',   Resource,   array('drw_wnd' => Resource));
f('DrawGetVectorGraphics',   String,     array('drw_wnd' => Resource));

f('DrawLine', NULL,
  array('drw_wnd' => Resource,
        'sx' => Double,
        'sy' => Double,
        'ex' => Double,
        'ey' => Double));

f('DrawMatte', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double,
        'paint_method' => Int32));

f('DrawPathClose', NULL,
  array('drw_wnd' => Resource));

f('DrawPathCurveToAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x2' => Double,
        'y2' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToQuadraticBezierAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToQuadraticBezierRelative', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToQuadraticBezierSmoothAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToQuadraticBezierSmoothRelative', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToRelative', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x2' => Double,
        'y2' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToSmoothAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x2' => Double,
        'y2' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathCurveToSmoothRelative', NULL,
  array('drw_wnd' => Resource,
        'x2' => Double,
        'y2' => Double,
        'x' => Double,
        'y' => Double));

f('DrawPathEllipticArcAbsolute', NULL,
  array('drw_wnd' => Resource,
        'rx' => Double,
        'ry' => Double,
        'x_axis_rotation' => Double,
        'large_arc_flag' => Boolean,
        'sweep_flag' => Boolean,
        'x' => Double,
        'y' => Double));

f('DrawPathEllipticArcRelative', NULL,
  array('drw_wnd' => Resource,
        'rx' => Double,
        'ry' => Double,
        'x_axis_rotation' => Double,
        'large_arc_flag' => Boolean,
        'sweep_flag' => Boolean,
        'x' => Double,
        'y' => Double));

f('DrawPathFinish', NULL,
  array('drw_wnd' => Resource));

f('DrawPathLineToAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathLineToHorizontalAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x' => Double));

f('DrawPathLineToHorizontalRelative', NULL,
  array('drw_wnd' => Resource,
        'x' => Double));

f('DrawPathLineToRelative', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathLineToVerticalAbsolute', NULL,
  array('drw_wnd' => Resource,
        'y' => Double));

f('DrawPathLineToVerticalRelative', NULL,
  array('drw_wnd' => Resource,
        'y' => Double));

f('DrawPathMoveToAbsolute', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathMoveToRelative', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPathStart', NULL,
  array('drw_wnd' => Resource));

f('DrawPoint', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawPolygon', NULL,
  array('drw_wnd' => Resource,
        'x_y_points_array' => VariantVec));

f('DrawPolyline', NULL,
  array('drw_wnd' => Resource,
        'x_y_points_array' => VariantVec));

f('DrawRectangle', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x2' => Double,
        'y2' => Double));

f('DrawRender', Boolean,
  array('drw_wnd' => Resource));

f('DrawRotate', NULL,
  array('drw_wnd' => Resource,
        'degrees' => Double));

f('DrawRoundRectangle', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x2' => Double,
        'y2' => Double,
        'rx' => Double,
        'ry' => Double));

f('DrawScale', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('DrawSetClipPath', Boolean,
  array('drw_wnd' => Resource,
        'clip_path' => String));

f('DrawSetClipRule', NULL,
  array('drw_wnd' => Resource,
        'fill_rule' => Int32));

f('DrawSetClipUnits', NULL,
  array('drw_wnd' => Resource,
        'clip_path_units' => Int32));

f('DrawSetFillAlpha', NULL,
  array('drw_wnd' => Resource,
        'fill_opacity' => Double));

f('DrawSetFillColor', NULL,
  array('drw_wnd' => Resource,
        'fill_pxl_wnd' => Resource));

f('DrawSetFillOpacity', NULL,
  array('drw_wnd' => Resource,
        'fill_opacity' => Double));

f('DrawSetFillPatternURL', Boolean,
  array('drw_wnd' => Resource,
        'fill_url' => String));

f('DrawSetFillRule', NULL,
  array('drw_wnd' => Resource,
        'fill_rule' => Int32));

f('DrawSetFont', Boolean,
  array('drw_wnd' => Resource,
        'font_file' => String));

f('DrawSetFontFamily', Boolean,
  array('drw_wnd' => Resource,
        'font_family' => String));

f('DrawSetFontSize', NULL,
  array('drw_wnd' => Resource,
        'pointsize' => Double));

f('DrawSetFontStretch', NULL,
  array('drw_wnd' => Resource,
        'stretch_type' => Int32));

f('DrawSetFontStyle', NULL,
  array('drw_wnd' => Resource,
        'style_type' => Int32));

f('DrawSetFontWeight', NULL,
  array('drw_wnd' => Resource,
        'font_weight' => Double));

f('DrawSetGravity', NULL,
  array('drw_wnd' => Resource,
        'gravity_type' => Int32));

f('DrawSetStrokeAlpha', NULL,
  array('drw_wnd' => Resource,
        'stroke_opacity' => Double));

f('DrawSetStrokeAntialias', NULL,
  array('drw_wnd' => Resource,
        'stroke_antialias' => array(Boolean, 'true')));

f('DrawSetStrokeColor', NULL,
  array('drw_wnd' => Resource,
        'strokecolor_pxl_wnd' => Resource));

f('DrawSetStrokeDashArray', NULL,
  array('drw_wnd' => Resource,
        'dash_array' => array(VariantVec, 'null_array')));

f('DrawSetStrokeDashOffset', NULL,
  array('drw_wnd' => Resource,
        'dash_offset' => Double));

f('DrawSetStrokeLineCap', NULL,
  array('drw_wnd' => Resource,
        'line_cap' => Int32));

f('DrawSetStrokeLineJoin', NULL,
  array('drw_wnd' => Resource,
        'line_join' => Int32));

f('DrawSetStrokeMiterLimit', NULL,
  array('drw_wnd' => Resource,
        'miterlimit' => Double));

f('DrawSetStrokeOpacity', NULL,
  array('drw_wnd' => Resource,
        'stroke_opacity' => Double));

f('DrawSetStrokePatternURL', Boolean,
  array('drw_wnd' => Resource,
        'stroke_url' => String));

f('DrawSetStrokeWidth', NULL,
  array('drw_wnd' => Resource,
        'stroke_width' => Double));

f('DrawSetTextAlignment', NULL,
  array('drw_wnd' => Resource,
        'align_type' => Int32));

f('DrawSetTextAntialias', NULL,
  array('drw_wnd' => Resource,
        'text_antialias' => array(Boolean, 'true')));

f('DrawSetTextDecoration', NULL,
  array('drw_wnd' => Resource,
        'decoration_type' => Int32));

f('DrawSetTextEncoding', NULL,
  array('drw_wnd' => Resource,
        'encoding' => String));

f('DrawSetTextUnderColor', NULL,
  array('drw_wnd' => Resource,
        'undercolor_pxl_wnd' => Resource));

f('DrawSetVectorGraphics', Boolean,
  array('drw_wnd' => Resource,
        'vector_graphics' => String));

f('DrawSetViewbox', NULL,
  array('drw_wnd' => Resource,
        'x1' => Double,
        'y1' => Double,
        'x2' => Double,
        'y2' => Double));

f('DrawSkewX', NULL,
  array('drw_wnd' => Resource,
        'degrees' => Double));

f('DrawSkewY', NULL,
  array('drw_wnd' => Resource,
        'degrees' => Double));

f('DrawTranslate', NULL,
  array('drw_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('PushDrawingWand', NULL,
  array('drw_wnd' => Resource));

f('DrawPushClipPath', NULL,
  array('drw_wnd' => Resource,
        'clip_path_id' => String));

f('DrawPushDefs', NULL,
  array('drw_wnd' => Resource));

f('DrawPushPattern', NULL,
  array('drw_wnd' => Resource,
        'pattern_id' => String,
        'x' => Double,
        'y' => Double,
        'width' => Double,
        'height' => Double));

f('PopDrawingWand', NULL,
  array('drw_wnd' => Resource));

f('DrawPopClipPath', NULL,
  array('drw_wnd' => Resource));

f('DrawPopDefs', NULL,
  array('drw_wnd' => Resource));

f('DrawPopPattern', NULL,
  array('drw_wnd' => Resource));

///////////////////////////////////////////////////////////////////////////////
// Magick Functions

f('MagickAdaptiveThresholdImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'offset' => Double));

f('MagickAddImage', Boolean,
  array('mgck_wnd' => Resource,
        'add_wand' => Resource));

f('MagickAddNoiseImage', Boolean,
  array('mgck_wnd' => Resource,
        'noise_type' => Int32));

f('MagickAffineTransformImage', Boolean,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource));

f('MagickAnnotateImage', Boolean,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'x' => Double,
        'y' => Double,
        'angle' => Double,
        'text' => String));

f('MagickAppendImages', Resource,
  array('mgck_wnd' => Resource,
        'stack_vertical' => array(Boolean, 'false')));

f('MagickAverageImages', Resource,
  array('mgck_wnd' => Resource));

f('MagickBlackThresholdImage', Boolean,
  array('mgck_wnd' => Resource,
        'threshold_pxl_wnd' => Resource));

f('MagickBlurImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickBorderImage', Boolean,
  array('mgck_wnd' => Resource,
        'bordercolor' => Resource,
        'width' => Double,
        'height' => Double));

f('MagickCharcoalImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double));

f('MagickChopImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'x' => Int32,
        'y' => Int32));

f('MagickClipImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickClipPathImage', Boolean,
  array('mgck_wnd' => Resource,
        'pathname' => String,
        'inside' => Boolean));

f('MagickCoalesceImages', Resource,
  array('mgck_wnd' => Resource));

f('MagickColorFloodfillImage', Boolean,
  array('mgck_wnd' => Resource,
        'fillcolor_pxl_wnd' => Resource,
        'fuzz' => Double,
        'bordercolor_pxl_wnd' => Resource,
        'x' => Int32,
        'y' => Int32));

f('MagickColorizeImage', Boolean,
  array('mgck_wnd' => Resource,
        'colorize' => Resource,
        'opacity_pxl_wnd' => Resource));

f('MagickCombineImages', Resource,
  array('mgck_wnd' => Resource,
        'channel_type' => Int32));

f('MagickCommentImage', Boolean,
  array('mgck_wnd' => Resource,
        'comment' => String));

f('MagickCompareImages', VariantVec,
  array('mgck_wnd' => Resource,
        'reference_wnd' => Resource,
        'metric_type' => Int32,
        'channel_type' => array(Int32, '0')));

f('MagickCompositeImage', Boolean,
  array('mgck_wnd' => Resource,
        'composite_wnd' => Resource,
        'composite_operator' => Int32,
        'x' => Int32,
        'y' => Int32));

f('MagickConstituteImage', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Double,
        'rows' => Double,
        'smap' => String,
        'storage_type' => Int32,
        'pixel_array' => VariantVec));

f('MagickContrastImage', Boolean,
  array('mgck_wnd' => Resource,
        'sharpen' => Boolean));

f('MagickConvolveImage', Boolean,
  array('mgck_wnd' => Resource,
        'kernel_array' => VariantVec,
        'channel_type' => array(Int32, '0')));

f('MagickCropImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'x' => Int32,
        'y' => Int32));

f('MagickCycleColormapImage', Boolean,
  array('mgck_wnd' => Resource,
        'num_positions' => Int32));

f('MagickDeconstructImages', Resource,
  array('mgck_wnd' => Resource));

f('MagickDescribeImage', String,
  array('mgck_wnd' => Resource));

f('MagickDespeckleImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickDrawImage', Boolean,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource));

f('MagickEchoImageBlob', Boolean,
  array('mgck_wnd' => Resource));

f('MagickEchoImagesBlob', Boolean,
  array('mgck_wnd' => Resource));

f('MagickEdgeImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double));

f('MagickEmbossImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double));

f('MagickEnhanceImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickEqualizeImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickEvaluateImage', Boolean,
  array('mgck_wnd' => Resource,
        'evaluate_op' => Int32,
        'constant' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickFlattenImages', Resource,
  array('mgck_wnd' => Resource));

f('MagickFlipImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickFlopImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickFrameImage', Boolean,
  array('mgck_wnd' => Resource,
        'matte_color' => Resource,
        'width' => Double,
        'height' => Double,
        'inner_bevel' => Int32,
        'outer_bevel' => Int32));

f('MagickFxImage', Resource,
  array('mgck_wnd' => Resource,
        'expression' => String,
        'channel_type' => array(Int32, '0')));

f('MagickGammaImage', Boolean,
  array('mgck_wnd' => Resource,
        'gamma' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickGaussianBlurImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickGetCharHeight', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetCharWidth', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetException', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetExceptionString', String,
  array('mgck_wnd' => Resource));

f('MagickGetExceptionType', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetFilename', String,
  array('mgck_wnd' => Resource));

f('MagickGetFormat', String,
  array('mgck_wnd' => Resource));

f('MagickGetImage', Resource,
  array('mgck_wnd' => Resource));

f('MagickGetImageBackgroundColor', Resource,
  array('mgck_wnd' => Resource));

f('MagickGetImageBlob', String,
  array('mgck_wnd' => Resource));

f('MagickGetImageBluePrimary', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageBorderColor', Resource,
  array('mgck_wnd' => Resource));

f('MagickGetImageChannelMean', VariantVec,
  array('mgck_wnd' => Resource,
        'channel_type' => Int32));

f('MagickGetImageColormapColor', Resource,
  array('mgck_wnd' => Resource,
        'index' => Double));

f('MagickGetImageColors', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageColorspace', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageCompose', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageCompression', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageCompressionQuality', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageDelay', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageDepth', Double,
  array('mgck_wnd' => Resource,
        'channel_type' => array(Int32, '0')));

f('MagickGetImageDispose', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageExtrema', VariantVec,
  array('mgck_wnd' => Resource,
        'channel_type' => array(Int32, '0')));

f('MagickGetImageFilename', String,
  array('mgck_wnd' => Resource));

f('MagickGetImageFormat', String,
  array('mgck_wnd' => Resource));

f('MagickGetImageGamma', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageGreenPrimary', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageHeight', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageHistogram', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageIndex', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageInterlaceScheme', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageIterations', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageMatteColor', Resource,
  array('mgck_wnd' => Resource));

f('MagickGetImageMimeType', String,
  array('mgck_wnd' => Resource));

f('MagickGetImagePixels', VariantVec,
  array('mgck_wnd' => Resource,
        'x_offset' => Int32,
        'y_offset' => Int32,
        'columns' => Double,
        'rows' => Double,
        'smap' => String,
        'storage_type' => Int32));

f('MagickGetImageProfile', String,
  array('mgck_wnd' => Resource,
        'name' => String));

f('MagickGetImageRedPrimary', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageRenderingIntent', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageResolution', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageScene', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImageSignature', String,
  array('mgck_wnd' => Resource));

f('MagickGetImageSize', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageType', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageUnits', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageVirtualPixelMethod', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetImageWhitePoint', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetImageWidth', Double,
  array('mgck_wnd' => Resource));

f('MagickGetImagesBlob', String,
  array('mgck_wnd' => Resource));

f('MagickGetInterlaceScheme', Int32,
  array('mgck_wnd' => Resource));

f('MagickGetMaxTextAdvance', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetMimeType', String,
  array('mgck_wnd' => Resource));

f('MagickGetNumberImages', Double,
  array('mgck_wnd' => Resource));

f('MagickGetSamplingFactors', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetSize', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickGetStringHeight', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetStringWidth', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetTextAscent', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetTextDescent', Double,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickGetWandSize', VariantVec,
  array('mgck_wnd' => Resource));

f('MagickHasNextImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickHasPreviousImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickImplodeImage', Boolean,
  array('mgck_wnd' => Resource,
        'amount' => Double));

f('MagickLabelImage', Boolean,
  array('mgck_wnd' => Resource,
        'label' => String));

f('MagickLevelImage', Boolean,
  array('mgck_wnd' => Resource,
        'black_point' => Double,
        'gamma' => Double,
        'white_point' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickMagnifyImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickMapImage', Boolean,
  array('mgck_wnd' => Resource,
        'map_wand' => Resource,
        'dither' => Boolean));

f('MagickMatteFloodfillImage', Boolean,
  array('mgck_wnd' => Resource,
        'opacity' => Double,
        'fuzz' => Double,
        'bordercolor_pxl_wnd' => Resource,
        'x' => Int32,
        'y' => Int32));

f('MagickMedianFilterImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double));

f('MagickMinifyImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickModulateImage', Boolean,
  array('mgck_wnd' => Resource,
        'brightness' => Double,
        'saturation' => Double,
        'hue' => Double));

f('MagickMontageImage', Resource,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'tile_geometry' => String,
        'thumbnail_geometry' => String,
        'montage_mode' => Int32,
        'frame' => String));

f('MagickMorphImages', Resource,
  array('mgck_wnd' => Resource,
        'number_frames' => Double));

f('MagickMosaicImages', Resource,
  array('mgck_wnd' => Resource));

f('MagickMotionBlurImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double,
        'angle' => Double));

f('MagickNegateImage', Boolean,
  array('mgck_wnd' => Resource,
        'only_the_gray' => array(Boolean, 'false'),
        'channel_type' => array(Int32, '0')));

f('MagickNewImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'imagemagick_col_str' => array(String, 'null_string')));

f('MagickNextImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickNormalizeImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickOilPaintImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double));

f('MagickPaintOpaqueImage', Boolean,
  array('mgck_wnd' => Resource,
        'target_pxl_wnd' => Resource,
        'fill_pxl_wnd' => Resource,
        'fuzz' => array(Double, '0.0')));

f('MagickPaintTransparentImage', Boolean,
  array('mgck_wnd' => Resource,
        'target' => Resource,
        'opacity' => array(Double, 'k_MW_TransparentOpacity'),
        'fuzz' => array(Double, '0.0')));

f('MagickPingImage', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => String));

f('MagickPosterizeImage', Boolean,
  array('mgck_wnd' => Resource,
        'levels' => Double,
        'dither' => Boolean));

f('MagickPreviewImages', Resource,
  array('mgck_wnd' => Resource,
        'preview' => Int32));

f('MagickPreviousImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickProfileImage', Boolean,
  array('mgck_wnd' => Resource,
        'name' => String,
        'profile' => array(String, 'null_string')));

f('MagickQuantizeImage', Boolean,
  array('mgck_wnd' => Resource,
        'number_colors' => Double,
        'colorspace_type' => Int32,
        'treedepth' => Double,
        'dither' => Boolean,
        'measure_error' => Boolean));

f('MagickQuantizeImages', Boolean,
  array('mgck_wnd' => Resource,
        'number_colors' => Double,
        'colorspace_type' => Int32,
        'treedepth' => Double,
        'dither' => Boolean,
        'measure_error' => Boolean));

f('MagickQueryFontMetrics', VariantVec,
  array('mgck_wnd' => Resource,
        'drw_wnd' => Resource,
        'txt' => String,
        'multiline' => array(Boolean, 'false')));

f('MagickRadialBlurImage', Boolean,
  array('mgck_wnd' => Resource,
        'angle' => Double));

f('MagickRaiseImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'x' => Int32,
        'y' => Int32,
        'raise' => Boolean));

f('MagickReadImage', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => String));

f('MagickReadImageBlob', Boolean,
  array('mgck_wnd' => Resource,
        'blob' => String));

f('MagickReadImageFile', Boolean,
  array('mgck_wnd' => Resource,
        'handle' => Resource));

f('MagickReadImages', Boolean,
  array('mgck_wnd' => Resource,
        'img_filenames_array' => StringVec));

f('MagickReduceNoiseImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double));

f('MagickRemoveImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickRemoveImageProfile', String,
  array('mgck_wnd' => Resource,
        'name' => String));

f('MagickRemoveImageProfiles', Boolean,
  array('mgck_wnd' => Resource));

f('MagickResampleImage', Boolean,
  array('mgck_wnd' => Resource,
        'x_resolution' => Double,
        'y_resolution' => Double,
        'filter_type' => Int32,
        'blur' => Double));

f('MagickResetIterator', NULL,
  array('mgck_wnd' => Resource));

f('MagickResizeImage', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Double,
        'rows' => Double,
        'filter_type' => Int32,
        'blur' => Double));

f('MagickRollImage', Boolean,
  array('mgck_wnd' => Resource,
        'x_offset' => Int32,
        'y_offset' => Int32));

f('MagickRotateImage', Boolean,
  array('mgck_wnd' => Resource,
        'background' => Resource,
        'degrees' => Double));

f('MagickSampleImage', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Double,
        'rows' => Double));

f('MagickScaleImage', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Double,
        'rows' => Double));

f('MagickSeparateImageChannel', Boolean,
  array('mgck_wnd' => Resource,
        'channel_type' => Int32));

f('MagickSetCompressionQuality', Boolean,
  array('mgck_wnd' => Resource,
        'quality' => Double));

f('MagickSetFilename', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => array(String, 'null_string')));

f('MagickSetFirstIterator', NULL,
  array('mgck_wnd' => Resource));

f('MagickSetFormat', Boolean,
  array('mgck_wnd' => Resource,
        'format' => String));

f('MagickSetImage', Boolean,
  array('mgck_wnd' => Resource,
        'replace_wand' => Resource));

f('MagickSetImageBackgroundColor', Boolean,
  array('mgck_wnd' => Resource,
        'background_pxl_wnd' => Resource));

f('MagickSetImageBias', Boolean,
  array('mgck_wnd' => Resource,
        'bias' => Double));

f('MagickSetImageBluePrimary', Boolean,
  array('mgck_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('MagickSetImageBorderColor', Boolean,
  array('mgck_wnd' => Resource,
        'border_pxl_wnd' => Resource));

f('MagickSetImageColormapColor', Boolean,
  array('mgck_wnd' => Resource,
        'index' => Double,
        'mapcolor_pxl_wnd' => Resource));

f('MagickSetImageColorspace', Boolean,
  array('mgck_wnd' => Resource,
        'colorspace_type' => Int32));

f('MagickSetImageCompose', Boolean,
  array('mgck_wnd' => Resource,
        'composite_operator' => Int32));

f('MagickSetImageCompression', Boolean,
  array('mgck_wnd' => Resource,
        'compression_type' => Int32));

f('MagickSetImageCompressionQuality', Boolean,
  array('mgck_wnd' => Resource,
        'quality' => Double));

f('MagickSetImageDelay', Boolean,
  array('mgck_wnd' => Resource,
        'delay' => Double));

f('MagickSetImageDepth', Boolean,
  array('mgck_wnd' => Resource,
        'depth' => Int32,
        'channel_type' => array(Int32, '0')));

f('MagickSetImageDispose', Boolean,
  array('mgck_wnd' => Resource,
        'dispose_type' => Int32));

f('MagickSetImageFilename', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => array(String, 'null_string')));

f('MagickSetImageFormat', Boolean,
  array('mgck_wnd' => Resource,
        'format' => String));

f('MagickSetImageGamma', Boolean,
  array('mgck_wnd' => Resource,
        'gamma' => Double));

f('MagickSetImageGreenPrimary', Boolean,
  array('mgck_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('MagickSetImageIndex', Boolean,
  array('mgck_wnd' => Resource,
        'index' => Int32));

f('MagickSetImageInterlaceScheme', Boolean,
  array('mgck_wnd' => Resource,
        'interlace_type' => Int32));

f('MagickSetImageIterations', Boolean,
  array('mgck_wnd' => Resource,
        'iterations' => Double));

f('MagickSetImageMatteColor', Boolean,
  array('mgck_wnd' => Resource,
        'matte_pxl_wnd' => Resource));

f('MagickSetImageOption', Boolean,
  array('mgck_wnd' => Resource,
        'format' => String,
        'key' => String,
        'value' => String));

f('MagickSetImagePixels', Boolean,
  array('mgck_wnd' => Resource,
        'x_offset' => Int32,
        'y_offset' => Int32,
        'columns' => Double,
        'rows' => Double,
        'smap' => String,
        'storage_type' => Int32,
        'pixel_array' => VariantVec));

f('MagickSetImageProfile', Boolean,
  array('mgck_wnd' => Resource,
        'name' => String,
        'profile' => String));

f('MagickSetImageRedPrimary', Boolean,
  array('mgck_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('MagickSetImageRenderingIntent', Boolean,
  array('mgck_wnd' => Resource,
        'rendering_intent' => Int32));

f('MagickSetImageResolution', Boolean,
  array('mgck_wnd' => Resource,
        'x_resolution' => Double,
        'y_resolution' => Double));

f('MagickSetImageScene', Boolean,
  array('mgck_wnd' => Resource,
        'scene' => Double));

f('MagickSetImageType', Boolean,
  array('mgck_wnd' => Resource,
        'image_type' => Int32));

f('MagickSetImageUnits', Boolean,
  array('mgck_wnd' => Resource,
        'resolution_type' => Int32));

f('MagickSetImageVirtualPixelMethod', Boolean,
  array('mgck_wnd' => Resource,
        'virtual_pixel_method' => Int32));

f('MagickSetImageWhitePoint', Boolean,
  array('mgck_wnd' => Resource,
        'x' => Double,
        'y' => Double));

f('MagickSetInterlaceScheme', Boolean,
  array('mgck_wnd' => Resource,
        'interlace_type' => Int32));

f('MagickSetLastIterator', NULL,
  array('mgck_wnd' => Resource));

f('MagickSetPassphrase', Boolean,
  array('mgck_wnd' => Resource,
        'passphrase' => String));

f('MagickSetResolution', Boolean,
  array('mgck_wnd' => Resource,
        'x_resolution' => Double,
        'y_resolution' => Double));

f('MagickSetSamplingFactors', Boolean,
  array('mgck_wnd' => Resource,
        'number_factors' => Double,
        'sampling_factors' => VariantVec));

f('MagickSetSize', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Int32,
        'rows' => Int32));

f('MagickSetWandSize', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Int32,
        'rows' => Int32));

f('MagickSharpenImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickShaveImage', Boolean,
  array('mgck_wnd' => Resource,
        'columns' => Int32,
        'rows' => Int32));

f('MagickShearImage', Boolean,
  array('mgck_wnd' => Resource,
        'background' => Resource,
        'x_shear' => Double,
        'y_shear' => Double));

f('MagickSolarizeImage', Boolean,
  array('mgck_wnd' => Resource,
        'threshold' => Double));

f('MagickSpliceImage', Boolean,
  array('mgck_wnd' => Resource,
        'width' => Double,
        'height' => Double,
        'x' => Int32,
        'y' => Int32));

f('MagickSpreadImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double));

f('MagickSteganoImage', Resource,
  array('mgck_wnd' => Resource,
        'watermark_wand' => Resource,
        'offset' => Int32));

f('MagickStereoImage', Boolean,
  array('mgck_wnd' => Resource,
        'offset_wand' => Resource));

f('MagickStripImage', Boolean,
  array('mgck_wnd' => Resource));

f('MagickSwirlImage', Boolean,
  array('mgck_wnd' => Resource,
        'degrees' => Double));

f('MagickTextureImage', Resource,
  array('mgck_wnd' => Resource,
        'texture_wand' => Resource));

f('MagickThresholdImage', Boolean,
  array('mgck_wnd' => Resource,
        'threshold' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickTintImage', Boolean,
  array('mgck_wnd' => Resource,
        'tint_pxl_wnd' => Int32,
        'opacity_pxl_wnd' => Resource));

f('MagickTransformImage', Resource,
  array('mgck_wnd' => Resource,
        'crop' => String,
        'geometry' => String));

f('MagickTrimImage', Boolean,
  array('mgck_wnd' => Resource,
        'fuzz' => Double));

f('MagickUnsharpMaskImage', Boolean,
  array('mgck_wnd' => Resource,
        'radius' => Double,
        'sigma' => Double,
        'amount' => Double,
        'threshold' => Double,
        'channel_type' => array(Int32, '0')));

f('MagickWaveImage', Boolean,
  array('mgck_wnd' => Resource,
        'amplitude' => Double,
        'wave_length' => Double));

f('MagickWhiteThresholdImage', Boolean,
  array('mgck_wnd' => Resource,
        'threshold_pxl_wnd' => Resource));

f('MagickWriteImage', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => String));

f('MagickWriteImageFile', Boolean,
  array('mgck_wnd' => Resource,
        'handle' => Resource));

f('MagickWriteImages', Boolean,
  array('mgck_wnd' => Resource,
        'filename' => array(String, '""'),
        'join_images' => array(Boolean, 'false')));

f('MagickWriteImagesFile', Boolean,
  array('mgck_wnd' => Resource,
        'handle' => Resource));

///////////////////////////////////////////////////////////////////////////////
// Pixel Functions

f('PixelGetAlpha',           Double,     array('pxl_wnd' => Resource));
f('PixelGetAlphaQuantum',    Double,     array('pxl_wnd' => Resource));
f('PixelGetBlack',           Double,     array('pxl_wnd' => Resource));
f('PixelGetBlackQuantum',    Double,     array('pxl_wnd' => Resource));
f('PixelGetBlue',            Double,     array('pxl_wnd' => Resource));
f('PixelGetBlueQuantum',     Double,     array('pxl_wnd' => Resource));
f('PixelGetColorAsString',   String,     array('pxl_wnd' => Resource));
f('PixelGetColorCount',      Double,     array('pxl_wnd' => Resource));
f('PixelGetCyan',            Double,     array('pxl_wnd' => Resource));
f('PixelGetCyanQuantum',     Double,     array('pxl_wnd' => Resource));
f('PixelGetException',       VariantVec, array('pxl_wnd' => Resource));
f('PixelGetExceptionString', String,     array('pxl_wnd' => Resource));
f('PixelGetExceptionType',   Int32,      array('pxl_wnd' => Resource));
f('PixelGetGreen',           Double,     array('pxl_wnd' => Resource));
f('PixelGetGreenQuantum',    Double,     array('pxl_wnd' => Resource));
f('PixelGetIndex',           Double,     array('pxl_wnd' => Resource));
f('PixelGetMagenta',         Double,     array('pxl_wnd' => Resource));
f('PixelGetMagentaQuantum',  Double,     array('pxl_wnd' => Resource));
f('PixelGetOpacity',         Double,     array('pxl_wnd' => Resource));
f('PixelGetOpacityQuantum',  Double,     array('pxl_wnd' => Resource));
f('PixelGetQuantumColor',    VariantMap, array('pxl_wnd' => Resource));
f('PixelGetRed',             Double,     array('pxl_wnd' => Resource));
f('PixelGetRedQuantum',      Double,     array('pxl_wnd' => Resource));
f('PixelGetYellow',          Double,     array('pxl_wnd' => Resource));
f('PixelGetYellowQuantum',   Double,     array('pxl_wnd' => Resource));

f('PixelSetAlpha', NULL,
  array('pxl_wnd' => Resource,
        'alpha' => Double));

f('PixelSetAlphaQuantum', NULL,
  array('pxl_wnd' => Resource,
        'alpha' => Double));

f('PixelSetBlack', NULL,
  array('pxl_wnd' => Resource,
        'black' => Double));

f('PixelSetBlackQuantum', NULL,
  array('pxl_wnd' => Resource,
        'black' => Double));

f('PixelSetBlue', NULL,
  array('pxl_wnd' => Resource,
        'blue' => Double));

f('PixelSetBlueQuantum', NULL,
  array('pxl_wnd' => Resource,
        'blue' => Double));

f('PixelSetColor', NULL,
  array('pxl_wnd' => Resource,
        'imagemagick_col_str' => String));

f('PixelSetColorCount', NULL,
  array('pxl_wnd' => Resource,
        'count' => Int32));

f('PixelSetCyan', NULL,
  array('pxl_wnd' => Resource,
        'cyan' => Double));

f('PixelSetCyanQuantum', NULL,
  array('pxl_wnd' => Resource,
        'cyan' => Double));

f('PixelSetGreen', NULL,
  array('pxl_wnd' => Resource,
        'green' => Double));

f('PixelSetGreenQuantum', NULL,
  array('pxl_wnd' => Resource,
        'green' => Double));

f('PixelSetIndex', NULL,
  array('pxl_wnd' => Resource,
        'index' => Double));

f('PixelSetMagenta', NULL,
  array('pxl_wnd' => Resource,
        'magenta' => Double));

f('PixelSetMagentaQuantum', NULL,
  array('pxl_wnd' => Resource,
        'magenta' => Double));

f('PixelSetOpacity', NULL,
  array('pxl_wnd' => Resource,
        'opacity' => Double));

f('PixelSetOpacityQuantum', NULL,
  array('pxl_wnd' => Resource,
        'opacity' => Double));

f('PixelSetQuantumColor', NULL,
  array('pxl_wnd' => Resource,
        'red' => Double,
        'green' => Double,
        'blue' => Double,
        'opacity' => array(Double, '0.0')));

f('PixelSetRed', NULL,
  array('pxl_wnd' => Resource,
        'red' => Double));

f('PixelSetRedQuantum', NULL,
  array('pxl_wnd' => Resource,
        'red' => Double));

f('PixelSetYellow', NULL,
  array('pxl_wnd' => Resource,
        'yellow' => Double));

f('PixelSetYellowQuantum', NULL,
  array('pxl_wnd' => Resource,
        'yellow' => Double));

///////////////////////////////////////////////////////////////////////////////
// Pixel Iterator Functions

f('PixelGetIteratorException', VariantVec,
  array('pxl_iter' => Resource));
f('PixelGetIteratorExceptionString', String,
  array('pxl_iter' => Resource));
f('PixelGetIteratorExceptionType', Int32,
  array('pxl_iter' => Resource));
f('PixelGetNextIteratorRow', VariantVec,
  array('pxl_iter' => Resource));
f('PixelResetIterator', NULL,
  array('pxl_iter' => Resource));
f('PixelSetIteratorRow', Boolean,
  array('pxl_iter' => Resource,
        'row' => Int32));
f('PixelSyncIterator', Boolean,
  array('pxl_iter' => Resource));
