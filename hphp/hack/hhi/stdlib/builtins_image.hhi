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

const IMAGETYPE_BMP = 6;
const IMAGETYPE_COUNT = 18;
const IMAGETYPE_GIF = 1;
const IMAGETYPE_IFF = 14;
const IMAGETYPE_JB2 = 12;
const IMAGETYPE_JP2 = 10;
const IMAGETYPE_JPC = 9;
const IMAGETYPE_JPEG = 2;
const IMAGETYPE_JPEG2000 = 9;
const IMAGETYPE_JPX = 11;
const IMAGETYPE_PNG = 3;
const IMAGETYPE_PSD = 5;
const IMAGETYPE_SWC = 13;
const IMAGETYPE_SWF = 4;
const IMAGETYPE_TIFF_II = 7;
const IMAGETYPE_TIFF_MM = 8;
const IMAGETYPE_UNKNOWN = 0;
const IMAGETYPE_WBMP = 15;
const IMAGETYPE_XBM = 16;
const IMAGETYPE_ICO = 17;

<<__PHPStdLib>>
function gd_info() { }
function getimagesize($filename, &$imageinfo = null) { }
<<__PHPStdLib>>
function image_type_to_extension($imagetype, $include_dot = true) { }
function image_type_to_mime_type($imagetype) { }
<<__PHPStdLib>>
function image2wbmp($image, $filename = null, $threshold = -1) { }
<<__PHPStdLib>>
function imageaffine($image, $affine = array(), $clip = array()) { }
<<__PHPStdLib>>
function imageaffinematrixconcat($m1, $m2) { }
<<__PHPStdLib>>
function imageaffinematrixget($type, $options = array()) { }
function imagealphablending($image, $blendmode) { }
<<__PHPStdLib>>
function imageantialias($image, $on) { }
<<__PHPStdLib>>
function imagearc($image, $cx, $cy, $width, $height, $start, $end, $color) { }
<<__PHPStdLib>>
function imagechar($image, $font, $x, $y, $c, $color) { }
<<__PHPStdLib>>
function imagecharup($image, $font, $x, $y, $c, $color) { }
function imagecolorallocate($image, $red, $green, $blue) { }
function imagecolorallocatealpha($image, $red, $green, $blue, $alpha) { }
function imagecolorat($image, $x, $y) { }
<<__PHPStdLib>>
function imagecolorclosest($image, $red, $green, $blue) { }
<<__PHPStdLib>>
function imagecolorclosestalpha($image, $red, $green, $blue, $alpha) { }
<<__PHPStdLib>>
function imagecolorclosesthwb($image, $red, $green, $blue) { }
<<__PHPStdLib>>
function imagecolordeallocate($image, $color) { }
<<__PHPStdLib>>
function imagecolorexact($image, $red, $green, $blue) { }
<<__PHPStdLib>>
function imagecolorexactalpha($image, $red, $green, $blue, $alpha) { }
<<__PHPStdLib>>
function imagecolormatch($image1, $image2) { }
<<__PHPStdLib>>
function imagecolorresolve($image, $red, $green, $blue) { }
<<__PHPStdLib>>
function imagecolorresolvealpha($image, $red, $green, $blue, $alpha) { }
<<__PHPStdLib>>
function imagecolorset($image, $index, $red, $green, $blue) { }
function imagecolorsforindex($image, $index) { }
<<__PHPStdLib>>
function imagecolorstotal($image) { }
function imagecolortransparent($image, $color = -1) { }
<<__PHPStdLib>>
function imageconvolution($image, $matrix, $div, $offset) { }
function imagecopy($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h) { }
function imagecopymerge($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) { }
<<__PHPStdLib>>
function imagecopymergegray($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) { }
function imagecopyresampled($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $dst_w, $dst_h, $src_w, $src_h) { }
function imagecopyresized($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $dst_w, $dst_h, $src_w, $src_h) { }
<<__PHPStdLib>>
function imagecreate($width, $height) { }
<<__PHPStdLib>>
function imagecreatefromgd2part($filename, $srcx, $srcy, $width, $height) { }
<<__PHPStdLib>>
function imagecreatefromgd($filename) { }
<<__PHPStdLib>>
function imagecreatefromgd2($filename) { }
function imagecreatefromgif($filename) { }
function imagecreatefromjpeg($filename) { }
function imagecreatefrompng($filename) { }
function imagecreatefromstring($data) { }
<<__PHPStdLib>>
function imagecreatefromwbmp($filename) { }
<<__PHPStdLib>>
function imagecreatefromxbm($filename) { }
function imagecreatefromxpm($filename) { }
function imagecreatetruecolor($width, $height) { }
function imagecrop($image, $rect) { }
<<__PHPStdLib>>
function imagecropauto($image, $mode = -1, $threshold = 0.5, $color = -1) { }
<<__PHPStdLib>>
function imagedashedline($image, $x1, $y1, $x2, $y2, $color) { }
function imagedestroy($image) { }
<<__PHPStdLib>>
function imageellipse($image, $cx, $cy, $width, $height, $color) { }
function imagefill($image, $x, $y, $color) { }
<<__PHPStdLib>>
function imagefilledarc($image, $cx, $cy, $width, $height, $start, $end, $color, $style) { }
function imagefilledellipse($image, $cx, $cy, $width, $height, $color) { }
<<__PHPStdLib>>
function imagefilledpolygon($image, $points, $num_points, $color) { }
function imagefilledrectangle($image, $x1, $y1, $x2, $y2, $color) { }
<<__PHPStdLib>>
function imagefilltoborder($image, $x, $y, $border, $color) { }
<<__PHPStdLib>>
function imagefilter($image, $filtertype, $arg1 = 0, $arg2 = 0, $arg3 = 0, $arg4 = 0) { }
<<__PHPStdLib>>
function imageflip($image, $mode = -1) { }
<<__PHPStdLib>>
function imagefontheight($font) { }
<<__PHPStdLib>>
function imagefontwidth($font) { }
<<__PHPStdLib>>
function imageftbbox($size, $angle, $font_file, $text, $extrainfo = null) { }
<<__PHPStdLib>>
function imagefttext($image, $size, $angle, $x, $y, $col, $font_file, $text, $extrainfo = null) { }
<<__PHPStdLib>>
function imagegammacorrect($image, $inputgamma, $outputgamma) { }
<<__PHPStdLib>>
function imagegd2($image, $filename = null, $chunk_size = 0, $type = 0) { }
<<__PHPStdLib>>
function imagegd($image, $filename = null) { }
function imagegif($image, $filename = null) { }
function imagegrabscreen() { }
function imagegrabwindow($window, $client_area = 0) { }
function imageinterlace($image, $interlace = 0) { }
<<__PHPStdLib>>
function imageistruecolor($image) { }
function imagejpeg($image, $filename = null, $quality = -1) { }
<<__PHPStdLib>>
function imagelayereffect($image, $effect) { }
<<__PHPStdLib>>
function imageline($image, $x1, $y1, $x2, $y2, $color) { }
<<__PHPStdLib>>
function imageloadfont($file) { }
<<__PHPStdLib>>
function imagepalettecopy($destination, $source) { }
function imagepng($image, $filename = null, $quality = -1, $filters = -1) { }
function imagewebp($image, $filename = null, $quality = 80) { }
<<__PHPStdLib>>
function imagepolygon($image, $points, $num_points, $color) { }
function imagepsbbox($text, $font, $size, $space = 0, $tightness = 0, $angle = 0.0) { }
function imagepsencodefont($font_index, $encodingfile) { }
function imagepsextendfont($font_index, $extend) { }
function imagepsfreefont($fontindex) { }
function imagepsloadfont($filename) { }
function imagepsslantfont($font_index, $slant) { }
function imagepstext($image, $text, $font, $size, $foreground, $background, $x, $y, $space = 0, $tightness = 0, $angle = 0.0, $antialias_steps = 0) { }
function imagerectangle($image, $x1, $y1, $x2, $y2, $color) { }
<<__PHPStdLib>>
function imagerotate($source_image, $angle, $bgd_color, $ignore_transparent = 0) { }
function imagesavealpha($image, $saveflag) { }
<<__PHPStdLib>>
function imagesetbrush($image, $brush) { }
function imagesetpixel($image, $x, $y, $color) { }
<<__PHPStdLib>>
function imagesetstyle($image, $style) { }
<<__PHPStdLib>>
function imagesetthickness($image, $thickness) { }
<<__PHPStdLib>>
function imagesettile($image, $tile) { }
function imagestring($image, $font, $x, $y, $str, $color) { }
<<__PHPStdLib>>
function imagestringup($image, $font, $x, $y, $str, $color) { }
function imagesx($image) { }
function imagesy($image) { }
function imagetruecolortopalette($image, $dither, $ncolors) { }
<<__PHPStdLib>>
function imagettfbbox($size, $angle, $fontfile, $text) { }
function imagettftext($image, $size, $angle, $x, $y, $color, $fontfile, $text) { }
<<__PHPStdLib>>
function imagetypes() { }
<<__PHPStdLib>>
function imagewbmp($image, $filename = null, $foreground = -1) { }
function imagexbm($image, $filename = null, $foreground = -1) { }
<<__PHPStdLib>>
function iptcembed($iptcdata, $jpeg_file_name, $spool = 0) { }
<<__PHPStdLib>>
function iptcparse($iptcblock) { }
<<__PHPStdLib>>
function jpeg2wbmp($jpegname, $wbmpname, $dest_height, $dest_width, $threshold) { }
<<__PHPStdLib>>
function png2wbmp($pngname, $wbmpname, $dest_height, $dest_width, $threshold) { }
function exif_imagetype($filename) { }
<<__PHPStdLib>>
function exif_read_data($filename, $sections = null, $arrays = false, $thumbnail = false) { }
function read_exif_data($filename, $sections = null, $arrays = false, $thumbnail = false) { }
<<__PHPStdLib>>
function exif_tagname($index) { }
<<__PHPStdLib>>
function exif_thumbnail($filename, &$width = null, &$height = null, &$imagetype = null) { }
