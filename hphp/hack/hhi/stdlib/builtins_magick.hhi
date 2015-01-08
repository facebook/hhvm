<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function magickgetcopyright() { }
function magickgethomeurl() { }
function magickgetpackagename() { }
function magickgetquantumdepth() { }
function magickgetreleasedate() { }
function magickgetresourcelimit($resource_type) { }
function magickgetversion() { }
function magickgetversionnumber() { }
function magickgetversionstring() { }
function magickqueryconfigureoption($option) { }
function magickqueryconfigureoptions($pattern) { }
function magickqueryfonts($pattern) { }
function magickqueryformats($pattern) { }
function magicksetresourcelimit($resource_type, $limit) { }
function newdrawingwand() { }
function newmagickwand() { }
function newpixeliterator($mgck_wnd) { }
function newpixelregioniterator($mgck_wnd, $x, $y, $columns, $rows) { }
function newpixelwand($imagemagick_col_str = null) { }
function newpixelwandarray($num_pxl_wnds) { }
function newpixelwands($num_pxl_wnds) { }
function destroydrawingwand($drw_wnd) { }
function destroymagickwand($mgck_wnd) { }
function destroypixeliterator($pxl_iter) { }
function destroypixelwand($pxl_wnd) { }
function destroypixelwandarray($pxl_wnd_array) { }
function destroypixelwands($pxl_wnd_array) { }
function isdrawingwand($var) { }
function ismagickwand($var) { }
function ispixeliterator($var) { }
function ispixelwand($var) { }
function cleardrawingwand($drw_wnd) { }
function clearmagickwand($mgck_wnd) { }
function clearpixeliterator($pxl_iter) { }
function clearpixelwand($pxl_wnd) { }
function clonedrawingwand($drw_wnd) { }
function clonemagickwand($mgck_wnd) { }
function wandgetexception($wnd) { }
function wandgetexceptionstring($wnd) { }
function wandgetexceptiontype($wnd) { }
function wandhasexception($wnd) { }
function drawaffine($drw_wnd, $sx, $sy, $rx, $ry, $tx, $ty) { }
function drawannotation($drw_wnd, $x, $y, $text) { }
function drawarc($drw_wnd, $sx, $sy, $ex, $ey, $sd, $ed) { }
function drawbezier($drw_wnd, $x_y_points_array) { }
function drawcircle($drw_wnd, $ox, $oy, $px, $py) { }
function drawcolor($drw_wnd, $x, $y, $paint_method) { }
function drawcomment($drw_wnd, $comment) { }
function drawcomposite($drw_wnd, $composite_operator, $x, $y, $width, $height, $mgck_wnd) { }
function drawellipse($drw_wnd, $ox, $oy, $rx, $ry, $start, $end) { }
function drawgetclippath($drw_wnd) { }
function drawgetcliprule($drw_wnd) { }
function drawgetclipunits($drw_wnd) { }
function drawgetexception($drw_wnd) { }
function drawgetexceptionstring($drw_wnd) { }
function drawgetexceptiontype($drw_wnd) { }
function drawgetfillalpha($drw_wnd) { }
function drawgetfillcolor($drw_wnd) { }
function drawgetfillopacity($drw_wnd) { }
function drawgetfillrule($drw_wnd) { }
function drawgetfont($drw_wnd) { }
function drawgetfontfamily($drw_wnd) { }
function drawgetfontsize($drw_wnd) { }
function drawgetfontstretch($drw_wnd) { }
function drawgetfontstyle($drw_wnd) { }
function drawgetfontweight($drw_wnd) { }
function drawgetgravity($drw_wnd) { }
function drawgetstrokealpha($drw_wnd) { }
function drawgetstrokeantialias($drw_wnd) { }
function drawgetstrokecolor($drw_wnd) { }
function drawgetstrokedasharray($drw_wnd) { }
function drawgetstrokedashoffset($drw_wnd) { }
function drawgetstrokelinecap($drw_wnd) { }
function drawgetstrokelinejoin($drw_wnd) { }
function drawgetstrokemiterlimit($drw_wnd) { }
function drawgetstrokeopacity($drw_wnd) { }
function drawgetstrokewidth($drw_wnd) { }
function drawgettextalignment($drw_wnd) { }
function drawgettextantialias($drw_wnd) { }
function drawgettextdecoration($drw_wnd) { }
function drawgettextencoding($drw_wnd) { }
function drawgettextundercolor($drw_wnd) { }
function drawgetvectorgraphics($drw_wnd) { }
function drawline($drw_wnd, $sx, $sy, $ex, $ey) { }
function drawmatte($drw_wnd, $x, $y, $paint_method) { }
function drawpathclose($drw_wnd) { }
function drawpathcurvetoabsolute($drw_wnd, $x1, $y1, $x2, $y2, $x, $y) { }
function drawpathcurvetoquadraticbezierabsolute($drw_wnd, $x1, $y1, $x, $y) { }
function drawpathcurvetoquadraticbezierrelative($drw_wnd, $x1, $y1, $x, $y) { }
function drawpathcurvetoquadraticbeziersmoothabsolute($drw_wnd, $x, $y) { }
function drawpathcurvetoquadraticbeziersmoothrelative($drw_wnd, $x, $y) { }
function drawpathcurvetorelative($drw_wnd, $x1, $y1, $x2, $y2, $x, $y) { }
function drawpathcurvetosmoothabsolute($drw_wnd, $x2, $y2, $x, $y) { }
function drawpathcurvetosmoothrelative($drw_wnd, $x2, $y2, $x, $y) { }
function drawpathellipticarcabsolute($drw_wnd, $rx, $ry, $x_axis_rotation, $large_arc_flag, $sweep_flag, $x, $y) { }
function drawpathellipticarcrelative($drw_wnd, $rx, $ry, $x_axis_rotation, $large_arc_flag, $sweep_flag, $x, $y) { }
function drawpathfinish($drw_wnd) { }
function drawpathlinetoabsolute($drw_wnd, $x, $y) { }
function drawpathlinetohorizontalabsolute($drw_wnd, $x) { }
function drawpathlinetohorizontalrelative($drw_wnd, $x) { }
function drawpathlinetorelative($drw_wnd, $x, $y) { }
function drawpathlinetoverticalabsolute($drw_wnd, $y) { }
function drawpathlinetoverticalrelative($drw_wnd, $y) { }
function drawpathmovetoabsolute($drw_wnd, $x, $y) { }
function drawpathmovetorelative($drw_wnd, $x, $y) { }
function drawpathstart($drw_wnd) { }
function drawpoint($drw_wnd, $x, $y) { }
function drawpolygon($drw_wnd, $x_y_points_array) { }
function drawpolyline($drw_wnd, $x_y_points_array) { }
function drawrectangle($drw_wnd, $x1, $y1, $x2, $y2) { }
function drawrender($drw_wnd) { }
function drawrotate($drw_wnd, $degrees) { }
function drawroundrectangle($drw_wnd, $x1, $y1, $x2, $y2, $rx, $ry) { }
function drawscale($drw_wnd, $x, $y) { }
function drawsetclippath($drw_wnd, $clip_path) { }
function drawsetcliprule($drw_wnd, $fill_rule) { }
function drawsetclipunits($drw_wnd, $clip_path_units) { }
function drawsetfillalpha($drw_wnd, $fill_opacity) { }
function drawsetfillcolor($drw_wnd, $fill_pxl_wnd) { }
function drawsetfillopacity($drw_wnd, $fill_opacity) { }
function drawsetfillpatternurl($drw_wnd, $fill_url) { }
function drawsetfillrule($drw_wnd, $fill_rule) { }
function drawsetfont($drw_wnd, $font_file) { }
function drawsetfontfamily($drw_wnd, $font_family) { }
function drawsetfontsize($drw_wnd, $pointsize) { }
function drawsetfontstretch($drw_wnd, $stretch_type) { }
function drawsetfontstyle($drw_wnd, $style_type) { }
function drawsetfontweight($drw_wnd, $font_weight) { }
function drawsetgravity($drw_wnd, $gravity_type) { }
function drawsetstrokealpha($drw_wnd, $stroke_opacity) { }
function drawsetstrokeantialias($drw_wnd, $stroke_antialias = true) { }
function drawsetstrokecolor($drw_wnd, $strokecolor_pxl_wnd) { }
function drawsetstrokedasharray($drw_wnd, $dash_array = null) { }
function drawsetstrokedashoffset($drw_wnd, $dash_offset) { }
function drawsetstrokelinecap($drw_wnd, $line_cap) { }
function drawsetstrokelinejoin($drw_wnd, $line_join) { }
function drawsetstrokemiterlimit($drw_wnd, $miterlimit) { }
function drawsetstrokeopacity($drw_wnd, $stroke_opacity) { }
function drawsetstrokepatternurl($drw_wnd, $stroke_url) { }
function drawsetstrokewidth($drw_wnd, $stroke_width) { }
function drawsettextalignment($drw_wnd, $align_type) { }
function drawsettextantialias($drw_wnd, $text_antialias = true) { }
function drawsettextdecoration($drw_wnd, $decoration_type) { }
function drawsettextencoding($drw_wnd, $encoding) { }
function drawsettextundercolor($drw_wnd, $undercolor_pxl_wnd) { }
function drawsetvectorgraphics($drw_wnd, $vector_graphics) { }
function drawsetviewbox($drw_wnd, $x1, $y1, $x2, $y2) { }
function drawskewx($drw_wnd, $degrees) { }
function drawskewy($drw_wnd, $degrees) { }
function drawtranslate($drw_wnd, $x, $y) { }
function pushdrawingwand($drw_wnd) { }
function drawpushclippath($drw_wnd, $clip_path_id) { }
function drawpushdefs($drw_wnd) { }
function drawpushpattern($drw_wnd, $pattern_id, $x, $y, $width, $height) { }
function popdrawingwand($drw_wnd) { }
function drawpopclippath($drw_wnd) { }
function drawpopdefs($drw_wnd) { }
function drawpoppattern($drw_wnd) { }
function magickadaptivethresholdimage($mgck_wnd, $width, $height, $offset) { }
function magickaddimage($mgck_wnd, $add_wand) { }
function magickaddnoiseimage($mgck_wnd, $noise_type) { }
function magickaffinetransformimage($mgck_wnd, $drw_wnd) { }
function magickannotateimage($mgck_wnd, $drw_wnd, $x, $y, $angle, $text) { }
function magickappendimages($mgck_wnd, $stack_vertical = false) { }
function magickaverageimages($mgck_wnd) { }
function magickblackthresholdimage($mgck_wnd, $threshold_pxl_wnd) { }
function magickblurimage($mgck_wnd, $radius, $sigma, $channel_type = 0) { }
function magickborderimage($mgck_wnd, $bordercolor, $width, $height) { }
function magickcharcoalimage($mgck_wnd, $radius, $sigma) { }
function magickchopimage($mgck_wnd, $width, $height, $x, $y) { }
function magickclipimage($mgck_wnd) { }
function magickclippathimage($mgck_wnd, $pathname, $inside) { }
function magickcoalesceimages($mgck_wnd) { }
function magickcolorfloodfillimage($mgck_wnd, $fillcolor_pxl_wnd, $fuzz, $bordercolor_pxl_wnd, $x, $y) { }
function magickcolorizeimage($mgck_wnd, $colorize, $opacity_pxl_wnd) { }
function magickcombineimages($mgck_wnd, $channel_type) { }
function magickcommentimage($mgck_wnd, $comment) { }
function magickcompareimages($mgck_wnd, $reference_wnd, $metric_type, $channel_type = 0) { }
function magickcompositeimage($mgck_wnd, $composite_wnd, $composite_operator, $x, $y) { }
function magickconstituteimage($mgck_wnd, $columns, $rows, $smap, $storage_type, $pixel_array) { }
function magickcontrastimage($mgck_wnd, $sharpen) { }
function magickconvolveimage($mgck_wnd, $kernel_array, $channel_type = 0) { }
function magickcropimage($mgck_wnd, $width, $height, $x, $y) { }
function magickcyclecolormapimage($mgck_wnd, $num_positions) { }
function magickdeconstructimages($mgck_wnd) { }
function magickdescribeimage($mgck_wnd) { }
function magickdespeckleimage($mgck_wnd) { }
function magickdrawimage($mgck_wnd, $drw_wnd) { }
function magickechoimageblob($mgck_wnd) { }
function magickechoimagesblob($mgck_wnd) { }
function magickedgeimage($mgck_wnd, $radius) { }
function magickembossimage($mgck_wnd, $radius, $sigma) { }
function magickenhanceimage($mgck_wnd) { }
function magickequalizeimage($mgck_wnd) { }
function magickevaluateimage($mgck_wnd, $evaluate_op, $constant, $channel_type = 0) { }
function magickflattenimages($mgck_wnd) { }
function magickflipimage($mgck_wnd) { }
function magickflopimage($mgck_wnd) { }
function magickframeimage($mgck_wnd, $matte_color, $width, $height, $inner_bevel, $outer_bevel) { }
function magickfximage($mgck_wnd, $expression, $channel_type = 0) { }
function magickgammaimage($mgck_wnd, $gamma, $channel_type = 0) { }
function magickgaussianblurimage($mgck_wnd, $radius, $sigma, $channel_type = 0) { }
function magickgetcharheight($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgetcharwidth($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgetexception($mgck_wnd) { }
function magickgetexceptionstring($mgck_wnd) { }
function magickgetexceptiontype($mgck_wnd) { }
function magickgetfilename($mgck_wnd) { }
function magickgetformat($mgck_wnd) { }
function magickgetimage($mgck_wnd) { }
function magickgetimagebackgroundcolor($mgck_wnd) { }
function magickgetimageblob($mgck_wnd) { }
function magickgetimageblueprimary($mgck_wnd) { }
function magickgetimagebordercolor($mgck_wnd) { }
function magickgetimagechannelmean($mgck_wnd, $channel_type) { }
function magickgetimagecolormapcolor($mgck_wnd, $index) { }
function magickgetimagecolors($mgck_wnd) { }
function magickgetimagecolorspace($mgck_wnd) { }
function magickgetimagecompose($mgck_wnd) { }
function magickgetimagecompression($mgck_wnd) { }
function magickgetimagecompressionquality($mgck_wnd) { }
function magickgetimagedelay($mgck_wnd) { }
function magickgetimagedepth($mgck_wnd, $channel_type = 0) { }
function magickgetimagedispose($mgck_wnd) { }
function magickgetimageextrema($mgck_wnd, $channel_type = 0) { }
function magickgetimagefilename($mgck_wnd) { }
function magickgetimageformat($mgck_wnd) { }
function magickgetimagegamma($mgck_wnd) { }
function magickgetimagegreenprimary($mgck_wnd) { }
function magickgetimageheight($mgck_wnd) { }
function magickgetimagehistogram($mgck_wnd) { }
function magickgetimageindex($mgck_wnd) { }
function magickgetimageinterlacescheme($mgck_wnd) { }
function magickgetimageiterations($mgck_wnd) { }
function magickgetimagemattecolor($mgck_wnd) { }
function magickgetimagemimetype($mgck_wnd) { }
function magickgetimagepixels($mgck_wnd, $x_offset, $y_offset, $columns, $rows, $smap, $storage_type) { }
function magickgetimageprofile($mgck_wnd, $name) { }
function magickgetimageredprimary($mgck_wnd) { }
function magickgetimagerenderingintent($mgck_wnd) { }
function magickgetimageresolution($mgck_wnd) { }
function magickgetimagescene($mgck_wnd) { }
function magickgetimagesignature($mgck_wnd) { }
function magickgetimagesize($mgck_wnd) { }
function magickgetimagetype($mgck_wnd) { }
function magickgetimageunits($mgck_wnd) { }
function magickgetimagevirtualpixelmethod($mgck_wnd) { }
function magickgetimagewhitepoint($mgck_wnd) { }
function magickgetimagewidth($mgck_wnd) { }
function magickgetimagesblob($mgck_wnd) { }
function magickgetinterlacescheme($mgck_wnd) { }
function magickgetmaxtextadvance($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgetmimetype($mgck_wnd) { }
function magickgetnumberimages($mgck_wnd) { }
function magickgetsamplingfactors($mgck_wnd) { }
function magickgetsize($mgck_wnd) { }
function magickgetstringheight($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgetstringwidth($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgettextascent($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgettextdescent($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickgetwandsize($mgck_wnd) { }
function magickhasnextimage($mgck_wnd) { }
function magickhaspreviousimage($mgck_wnd) { }
function magickimplodeimage($mgck_wnd, $amount) { }
function magicklabelimage($mgck_wnd, $label) { }
function magicklevelimage($mgck_wnd, $black_point, $gamma, $white_point, $channel_type = 0) { }
function magickmagnifyimage($mgck_wnd) { }
function magickmapimage($mgck_wnd, $map_wand, $dither) { }
function magickmattefloodfillimage($mgck_wnd, $opacity, $fuzz, $bordercolor_pxl_wnd, $x, $y) { }
function magickmedianfilterimage($mgck_wnd, $radius) { }
function magickminifyimage($mgck_wnd) { }
function magickmodulateimage($mgck_wnd, $brightness, $saturation, $hue) { }
function magickmontageimage($mgck_wnd, $drw_wnd, $tile_geometry, $thumbnail_geometry, $montage_mode, $frame) { }
function magickmorphimages($mgck_wnd, $number_frames) { }
function magickmosaicimages($mgck_wnd) { }
function magickmotionblurimage($mgck_wnd, $radius, $sigma, $angle) { }
function magicknegateimage($mgck_wnd, $only_the_gray = false, $channel_type = 0) { }
function magicknewimage($mgck_wnd, $width, $height, $imagemagick_col_str = null) { }
function magicknextimage($mgck_wnd) { }
function magicknormalizeimage($mgck_wnd) { }
function magickoilpaintimage($mgck_wnd, $radius) { }
function magickpaintopaqueimage($mgck_wnd, $target_pxl_wnd, $fill_pxl_wnd, $fuzz = 0.0) { }
function magickpainttransparentimage($mgck_wnd, $target, $opacity = MW_TransparentOpacity, $fuzz = 0.0) { }
function magickpingimage($mgck_wnd, $filename) { }
function magickposterizeimage($mgck_wnd, $levels, $dither) { }
function magickpreviewimages($mgck_wnd, $preview) { }
function magickpreviousimage($mgck_wnd) { }
function magickprofileimage($mgck_wnd, $name, $profile = null) { }
function magickquantizeimage($mgck_wnd, $number_colors, $colorspace_type, $treedepth, $dither, $measure_error) { }
function magickquantizeimages($mgck_wnd, $number_colors, $colorspace_type, $treedepth, $dither, $measure_error) { }
function magickqueryfontmetrics($mgck_wnd, $drw_wnd, $txt, $multiline = false) { }
function magickradialblurimage($mgck_wnd, $angle) { }
function magickraiseimage($mgck_wnd, $width, $height, $x, $y, $raise) { }
function magickreadimage($mgck_wnd, $filename) { }
function magickreadimageblob($mgck_wnd, $blob) { }
function magickreadimagefile($mgck_wnd, $handle) { }
function magickreadimages($mgck_wnd, $img_filenames_array) { }
function magickreducenoiseimage($mgck_wnd, $radius) { }
function magickremoveimage($mgck_wnd) { }
function magickremoveimageprofile($mgck_wnd, $name) { }
function magickremoveimageprofiles($mgck_wnd) { }
function magickresampleimage($mgck_wnd, $x_resolution, $y_resolution, $filter_type, $blur) { }
function magickresetiterator($mgck_wnd) { }
function magickresizeimage($mgck_wnd, $columns, $rows, $filter_type, $blur) { }
function magickrollimage($mgck_wnd, $x_offset, $y_offset) { }
function magickrotateimage($mgck_wnd, $background, $degrees) { }
function magicksampleimage($mgck_wnd, $columns, $rows) { }
function magickscaleimage($mgck_wnd, $columns, $rows) { }
function magickseparateimagechannel($mgck_wnd, $channel_type) { }
function magicksetcompressionquality($mgck_wnd, $quality) { }
function magicksetfilename($mgck_wnd, $filename = null) { }
function magicksetfirstiterator($mgck_wnd) { }
function magicksetformat($mgck_wnd, $format) { }
function magicksetimage($mgck_wnd, $replace_wand) { }
function magicksetimagebackgroundcolor($mgck_wnd, $background_pxl_wnd) { }
function magicksetimagebias($mgck_wnd, $bias) { }
function magicksetimageblueprimary($mgck_wnd, $x, $y) { }
function magicksetimagebordercolor($mgck_wnd, $border_pxl_wnd) { }
function magicksetimagecolormapcolor($mgck_wnd, $index, $mapcolor_pxl_wnd) { }
function magicksetimagecolorspace($mgck_wnd, $colorspace_type) { }
function magicksetimagecompose($mgck_wnd, $composite_operator) { }
function magicksetimagecompression($mgck_wnd, $compression_type) { }
function magicksetimagecompressionquality($mgck_wnd, $quality) { }
function magicksetimagedelay($mgck_wnd, $delay) { }
function magicksetimagedepth($mgck_wnd, $depth, $channel_type = 0) { }
function magicksetimagedispose($mgck_wnd, $dispose_type) { }
function magicksetimagefilename($mgck_wnd, $filename = null) { }
function magicksetimageformat($mgck_wnd, $format) { }
function magicksetimagegamma($mgck_wnd, $gamma) { }
function magicksetimagegreenprimary($mgck_wnd, $x, $y) { }
function magicksetimageindex($mgck_wnd, $index) { }
function magicksetimageinterlacescheme($mgck_wnd, $interlace_type) { }
function magicksetimageiterations($mgck_wnd, $iterations) { }
function magicksetimagemattecolor($mgck_wnd, $matte_pxl_wnd) { }
function magicksetimageoption($mgck_wnd, $format, $key, $value) { }
function magicksetimagepixels($mgck_wnd, $x_offset, $y_offset, $columns, $rows, $smap, $storage_type, $pixel_array) { }
function magicksetimageprofile($mgck_wnd, $name, $profile) { }
function magicksetimageredprimary($mgck_wnd, $x, $y) { }
function magicksetimagerenderingintent($mgck_wnd, $rendering_intent) { }
function magicksetimageresolution($mgck_wnd, $x_resolution, $y_resolution) { }
function magicksetimagescene($mgck_wnd, $scene) { }
function magicksetimagetype($mgck_wnd, $image_type) { }
function magicksetimageunits($mgck_wnd, $resolution_type) { }
function magicksetimagevirtualpixelmethod($mgck_wnd, $virtual_pixel_method) { }
function magicksetimagewhitepoint($mgck_wnd, $x, $y) { }
function magicksetinterlacescheme($mgck_wnd, $interlace_type) { }
function magicksetlastiterator($mgck_wnd) { }
function magicksetpassphrase($mgck_wnd, $passphrase) { }
function magicksetresolution($mgck_wnd, $x_resolution, $y_resolution) { }
function magicksetsamplingfactors($mgck_wnd, $number_factors, $sampling_factors) { }
function magicksetsize($mgck_wnd, $columns, $rows) { }
function magicksetwandsize($mgck_wnd, $columns, $rows) { }
function magicksharpenimage($mgck_wnd, $radius, $sigma, $channel_type = 0) { }
function magickshaveimage($mgck_wnd, $columns, $rows) { }
function magickshearimage($mgck_wnd, $background, $x_shear, $y_shear) { }
function magicksolarizeimage($mgck_wnd, $threshold) { }
function magickspliceimage($mgck_wnd, $width, $height, $x, $y) { }
function magickspreadimage($mgck_wnd, $radius) { }
function magicksteganoimage($mgck_wnd, $watermark_wand, $offset) { }
function magickstereoimage($mgck_wnd, $offset_wand) { }
function magickstripimage($mgck_wnd) { }
function magickswirlimage($mgck_wnd, $degrees) { }
function magicktextureimage($mgck_wnd, $texture_wand) { }
function magickthresholdimage($mgck_wnd, $threshold, $channel_type = 0) { }
function magicktintimage($mgck_wnd, $tint_pxl_wnd, $opacity_pxl_wnd) { }
function magicktransformimage($mgck_wnd, $crop, $geometry) { }
function magicktrimimage($mgck_wnd, $fuzz) { }
function magickunsharpmaskimage($mgck_wnd, $radius, $sigma, $amount, $threshold, $channel_type = 0) { }
function magickwaveimage($mgck_wnd, $amplitude, $wave_length) { }
function magickwhitethresholdimage($mgck_wnd, $threshold_pxl_wnd) { }
function magickwriteimage($mgck_wnd, $filename) { }
function magickwriteimagefile($mgck_wnd, $handle) { }
function magickwriteimages($mgck_wnd, $filename = "", $join_images = false) { }
function magickwriteimagesfile($mgck_wnd, $handle) { }
function pixelgetalpha($pxl_wnd) { }
function pixelgetalphaquantum($pxl_wnd) { }
function pixelgetblack($pxl_wnd) { }
function pixelgetblackquantum($pxl_wnd) { }
function pixelgetblue($pxl_wnd) { }
function pixelgetbluequantum($pxl_wnd) { }
function pixelgetcolorasstring($pxl_wnd) { }
function pixelgetcolorcount($pxl_wnd) { }
function pixelgetcyan($pxl_wnd) { }
function pixelgetcyanquantum($pxl_wnd) { }
function pixelgetexception($pxl_wnd) { }
function pixelgetexceptionstring($pxl_wnd) { }
function pixelgetexceptiontype($pxl_wnd) { }
function pixelgetgreen($pxl_wnd) { }
function pixelgetgreenquantum($pxl_wnd) { }
function pixelgetindex($pxl_wnd) { }
function pixelgetmagenta($pxl_wnd) { }
function pixelgetmagentaquantum($pxl_wnd) { }
function pixelgetopacity($pxl_wnd) { }
function pixelgetopacityquantum($pxl_wnd) { }
function pixelgetquantumcolor($pxl_wnd) { }
function pixelgetred($pxl_wnd) { }
function pixelgetredquantum($pxl_wnd) { }
function pixelgetyellow($pxl_wnd) { }
function pixelgetyellowquantum($pxl_wnd) { }
function pixelsetalpha($pxl_wnd, $alpha) { }
function pixelsetalphaquantum($pxl_wnd, $alpha) { }
function pixelsetblack($pxl_wnd, $black) { }
function pixelsetblackquantum($pxl_wnd, $black) { }
function pixelsetblue($pxl_wnd, $blue) { }
function pixelsetbluequantum($pxl_wnd, $blue) { }
function pixelsetcolor($pxl_wnd, $imagemagick_col_str) { }
function pixelsetcolorcount($pxl_wnd, $count) { }
function pixelsetcyan($pxl_wnd, $cyan) { }
function pixelsetcyanquantum($pxl_wnd, $cyan) { }
function pixelsetgreen($pxl_wnd, $green) { }
function pixelsetgreenquantum($pxl_wnd, $green) { }
function pixelsetindex($pxl_wnd, $index) { }
function pixelsetmagenta($pxl_wnd, $magenta) { }
function pixelsetmagentaquantum($pxl_wnd, $magenta) { }
function pixelsetopacity($pxl_wnd, $opacity) { }
function pixelsetopacityquantum($pxl_wnd, $opacity) { }
function pixelsetquantumcolor($pxl_wnd, $red, $green, $blue, $opacity = 0.0) { }
function pixelsetred($pxl_wnd, $red) { }
function pixelsetredquantum($pxl_wnd, $red) { }
function pixelsetyellow($pxl_wnd, $yellow) { }
function pixelsetyellowquantum($pxl_wnd, $yellow) { }
function pixelgetiteratorexception($pxl_iter) { }
function pixelgetiteratorexceptionstring($pxl_iter) { }
function pixelgetiteratorexceptiontype($pxl_iter) { }
function pixelgetnextiteratorrow($pxl_iter) { }
function pixelresetiterator($pxl_iter) { }
function pixelsetiteratorrow($pxl_iter, $row) { }
function pixelsynciterator($pxl_iter) { }
