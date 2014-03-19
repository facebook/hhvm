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
function gd_info() { }
function getimagesize($filename, &$imageinfo = null) { }
function image_type_to_extension($imagetype, $include_dot = true) { }
function image_type_to_mime_type($imagetype) { }
function image2wbmp($image, $filename = null, $threshold = -1) { }
function imagealphablending($image, $blendmode) { }
function imageantialias($image, $on) { }
function imagearc($image, $cx, $cy, $width, $height, $start, $end, $color) { }
function imagechar($image, $font, $x, $y, $c, $color) { }
function imagecharup($image, $font, $x, $y, $c, $color) { }
function imagecolorallocate($image, $red, $green, $blue) { }
function imagecolorallocatealpha($image, $red, $green, $blue, $alpha) { }
function imagecolorat($image, $x, $y) { }
function imagecolorclosest($image, $red, $green, $blue) { }
function imagecolorclosestalpha($image, $red, $green, $blue, $alpha) { }
function imagecolorclosesthwb($image, $red, $green, $blue) { }
function imagecolordeallocate($image, $color) { }
function imagecolorexact($image, $red, $green, $blue) { }
function imagecolorexactalpha($image, $red, $green, $blue, $alpha) { }
function imagecolormatch($image1, $image2) { }
function imagecolorresolve($image, $red, $green, $blue) { }
function imagecolorresolvealpha($image, $red, $green, $blue, $alpha) { }
function imagecolorset($image, $index, $red, $green, $blue) { }
function imagecolorsforindex($image, $index) { }
function imagecolorstotal($image) { }
function imagecolortransparent($image, $color = -1) { }
function imageconvolution($image, $matrix, $div, $offset) { }
function imagecopy($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h) { }
function imagecopymerge($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) { }
function imagecopymergegray($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) { }
function imagecopyresampled($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $dst_w, $dst_h, $src_w, $src_h) { }
function imagecopyresized($dst_im, $src_im, $dst_x, $dst_y, $src_x, $src_y, $dst_w, $dst_h, $src_w, $src_h) { }
function imagecreate($width, $height) { }
function imagecreatefromgd2part($filename, $srcx, $srcy, $width, $height) { }
function imagecreatefromgd($filename) { }
function imagecreatefromgd2($filename) { }
function imagecreatefromgif($filename) { }
function imagecreatefromjpeg($filename) { }
function imagecreatefrompng($filename) { }
function imagecreatefromstring($data) { }
function imagecreatefromwbmp($filename) { }
function imagecreatefromxbm($filename) { }
function imagecreatefromxpm($filename) { }
function imagecreatetruecolor($width, $height) { }
function imagedashedline($image, $x1, $y1, $x2, $y2, $color) { }
function imagedestroy($image) { }
function imageellipse($image, $cx, $cy, $width, $height, $color) { }
function imagefill($image, $x, $y, $color) { }
function imagefilledarc($image, $cx, $cy, $width, $height, $start, $end, $color, $style) { }
function imagefilledellipse($image, $cx, $cy, $width, $height, $color) { }
function imagefilledpolygon($image, $points, $num_points, $color) { }
function imagefilledrectangle($image, $x1, $y1, $x2, $y2, $color) { }
function imagefilltoborder($image, $x, $y, $border, $color) { }
function imagefilter($image, $filtertype, $arg1 = 0, $arg2 = 0, $arg3 = 0, $arg4 = 0) { }
function imagefontheight($font) { }
function imagefontwidth($font) { }
function imageftbbox($size, $angle, $font_file, $text, $extrainfo = null) { }
function imagefttext($image, $size, $angle, $x, $y, $col, $font_file, $text, $extrainfo = null) { }
function imagegammacorrect($image, $inputgamma, $outputgamma) { }
function imagegd2($image, $filename = null, $chunk_size = 0, $type = 0) { }
function imagegd($image, $filename = null) { }
function imagegif($image, $filename = null) { }
function imagegrabscreen() { }
function imagegrabwindow($window, $client_area = 0) { }
function imageinterlace($image, $interlace = 0) { }
function imageistruecolor($image) { }
function imagejpeg($image, $filename = null, $quality = -1) { }
function imagelayereffect($image, $effect) { }
function imageline($image, $x1, $y1, $x2, $y2, $color) { }
function imageloadfont($file) { }
function imagepalettecopy($destination, $source) { }
function imagepng($image, $filename = null, $quality = -1, $filters = -1) { }
function imagepolygon($image, $points, $num_points, $color) { }
function imagepsbbox($text, $font, $size, $space = 0, $tightness = 0, $angle = 0.0) { }
function imagepsencodefont($font_index, $encodingfile) { }
function imagepsextendfont($font_index, $extend) { }
function imagepsfreefont($fontindex) { }
function imagepsloadfont($filename) { }
function imagepsslantfont($font_index, $slant) { }
function imagepstext($image, $text, $font, $size, $foreground, $background, $x, $y, $space = 0, $tightness = 0, $angle = 0.0, $antialias_steps = 0) { }
function imagerectangle($image, $x1, $y1, $x2, $y2, $color) { }
function imagerotate($source_image, $angle, $bgd_color, $ignore_transparent = 0) { }
function imagesavealpha($image, $saveflag) { }
function imagesetbrush($image, $brush) { }
function imagesetpixel($image, $x, $y, $color) { }
function imagesetstyle($image, $style) { }
function imagesetthickness($image, $thickness) { }
function imagesettile($image, $tile) { }
function imagestring($image, $font, $x, $y, $str, $color) { }
function imagestringup($image, $font, $x, $y, $str, $color) { }
function imagesx($image) { }
function imagesy($image) { }
function imagetruecolortopalette($image, $dither, $ncolors) { }
function imagettfbbox($size, $angle, $fontfile, $text) { }
function imagettftext($image, $size, $angle, $x, $y, $color, $fontfile, $text) { }
function imagetypes() { }
function imagewbmp($image, $filename = null, $foreground = -1) { }
function imagexbm($image, $filename = null, $foreground = -1) { }
function iptcembed($iptcdata, $jpeg_file_name, $spool = 0) { }
function iptcparse($iptcblock) { }
function jpeg2wbmp($jpegname, $wbmpname, $dest_height, $dest_width, $threshold) { }
function png2wbmp($pngname, $wbmpname, $dest_height, $dest_width, $threshold) { }
function exif_imagetype($filename) { }
function exif_read_data($filename, $sections = null, $arrays = false, $thumbnail = false) { }
function read_exif_data($filename, $sections = null, $arrays = false, $thumbnail = false) { }
function exif_tagname($index) { }
function exif_thumbnail($filename, &$width = null, &$height = null, &$imagetype = null) { }
