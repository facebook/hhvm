<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int IMAGETYPE_BMP;
const int IMAGETYPE_COUNT;
const int IMAGETYPE_GIF;
const int IMAGETYPE_IFF;
const int IMAGETYPE_JB2;
const int IMAGETYPE_JP2;
const int IMAGETYPE_JPC;
const int IMAGETYPE_JPEG;
const int IMAGETYPE_JPEG2000;
const int IMAGETYPE_JPX;
const int IMAGETYPE_PNG;
const int IMAGETYPE_PSD;
const int IMAGETYPE_SWC;
const int IMAGETYPE_SWF;
const int IMAGETYPE_TIFF_II;
const int IMAGETYPE_TIFF_MM;
const int IMAGETYPE_UNKNOWN;
const int IMAGETYPE_WBMP;
const int IMAGETYPE_XBM;
const int IMAGETYPE_ICO;
const int IMAGETYPE_WEBP;

<<__PHPStdLib>>
function gd_info(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getimagesize(
  string $filename,
  inout mixed $imageinfo,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function image_type_to_extension(
  int $imagetype,
  bool $include_dot = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function image_type_to_mime_type(int $imagetype): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function image2wbmp(
  resource $image,
  string $filename = "",
  int $threshold = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageaffine(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $affine = vec[],
  HH\FIXME\MISSING_PARAM_TYPE $clip = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageaffinematrixconcat(
  HH\FIXME\MISSING_PARAM_TYPE $m1,
  HH\FIXME\MISSING_PARAM_TYPE $m2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageaffinematrixget(
  int $type,
  HH\FIXME\MISSING_PARAM_TYPE $options = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagealphablending(
  resource $image,
  bool $blendmode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageantialias(
  resource $image,
  bool $on,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagearc(
  resource $image,
  int $cx,
  int $cy,
  int $width,
  int $height,
  int $start,
  int $end,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagechar(
  resource $image,
  int $font,
  int $x,
  int $y,
  string $c,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecharup(
  resource $image,
  int $font,
  int $x,
  int $y,
  string $c,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorallocate(
  resource $image,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorallocatealpha(
  resource $image,
  int $red,
  int $green,
  int $blue,
  int $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorat(
  resource $image,
  int $x,
  int $y,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorclosest(
  resource $image,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorclosestalpha(
  resource $image,
  int $red,
  int $green,
  int $blue,
  int $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorclosesthwb(
  resource $image,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolordeallocate(
  resource $image,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorexact(
  resource $image,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorexactalpha(
  resource $image,
  int $red,
  int $green,
  int $blue,
  int $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolormatch(
  resource $image1,
  resource $image2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorresolve(
  resource $image,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorresolvealpha(
  resource $image,
  int $red,
  int $green,
  int $blue,
  int $alpha,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorset(
  resource $image,
  int $index,
  int $red,
  int $green,
  int $blue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorsforindex(
  resource $image,
  int $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolorstotal(resource $image): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecolortransparent(
  resource $image,
  int $color = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageconvolution(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $matrix,
  float $div,
  float $offset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecopy(
  resource $dst_im,
  resource $src_im,
  int $dst_x,
  int $dst_y,
  int $src_x,
  int $src_y,
  int $src_w,
  int $src_h,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecopymerge(
  resource $dst_im,
  resource $src_im,
  int $dst_x,
  int $dst_y,
  int $src_x,
  int $src_y,
  int $src_w,
  int $src_h,
  int $pct,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecopymergegray(
  resource $dst_im,
  resource $src_im,
  int $dst_x,
  int $dst_y,
  int $src_x,
  int $src_y,
  int $src_w,
  int $src_h,
  int $pct,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecopyresampled(
  resource $dst_im,
  resource $src_im,
  int $dst_x,
  int $dst_y,
  int $src_x,
  int $src_y,
  int $dst_w,
  int $dst_h,
  int $src_w,
  int $src_h,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecopyresized(
  resource $dst_im,
  resource $src_im,
  int $dst_x,
  int $dst_y,
  int $src_x,
  int $src_y,
  int $dst_w,
  int $dst_h,
  int $src_w,
  int $src_h,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreate(int $width, int $height): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromgd2part(
  string $filename,
  int $srcx,
  int $srcy,
  int $width,
  int $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromgd(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromgd2(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromgif(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromjpeg(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefrompng(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromstring(string $data): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromwbmp(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatefromxbm(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecreatetruecolor(
  int $width,
  int $height,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecrop(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $rect,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagecropauto(
  resource $image,
  int $mode = -1,
  float $threshold = 0.5,
  int $color = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagedashedline(
  resource $image,
  int $x1,
  int $y1,
  int $x2,
  int $y2,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagedestroy(resource $image): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageellipse(
  resource $image,
  int $cx,
  int $cy,
  int $width,
  int $height,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefill(
  resource $image,
  int $x,
  int $y,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilledarc(
  resource $image,
  int $cx,
  int $cy,
  int $width,
  int $height,
  int $start,
  int $end,
  int $color,
  int $style,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilledellipse(
  resource $image,
  int $cx,
  int $cy,
  int $width,
  int $height,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilledpolygon(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $points,
  int $num_points,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilledrectangle(
  resource $image,
  int $x1,
  int $y1,
  int $x2,
  int $y2,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilltoborder(
  resource $image,
  int $x,
  int $y,
  int $border,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefilter(
  resource $image,
  int $filtertype,
  HH\FIXME\MISSING_PARAM_TYPE $arg1 = 0,
  HH\FIXME\MISSING_PARAM_TYPE $arg2 = 0,
  HH\FIXME\MISSING_PARAM_TYPE $arg3 = 0,
  HH\FIXME\MISSING_PARAM_TYPE $arg4 = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageflip(
  resource $image,
  int $mode = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefontheight(int $font): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefontwidth(int $font): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageftbbox(
  float $size,
  float $angle,
  string $font_file,
  string $text,
  HH\FIXME\MISSING_PARAM_TYPE $extrainfo = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagefttext(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $size,
  HH\FIXME\MISSING_PARAM_TYPE $angle,
  int $x,
  int $y,
  int $col,
  string $font_file,
  string $text,
  HH\FIXME\MISSING_PARAM_TYPE $extrainfo = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegammacorrect(
  resource $image,
  float $inputgamma,
  float $outputgamma,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegd2(
  resource $image,
  string $filename = "",
  int $chunk_size = 0,
  int $type = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegd(
  resource $image,
  string $filename = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegif(
  resource $image,
  string $filename = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegrabscreen(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagegrabwindow(
  HH\FIXME\MISSING_PARAM_TYPE $window,
  HH\FIXME\MISSING_PARAM_TYPE $client_area = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageinterlace(
  resource $image,
  ?int $interlace = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageistruecolor(resource $image): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagejpeg(
  resource $image,
  string $filename = "",
  int $quality = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagelayereffect(
  resource $image,
  int $effect,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageline(
  resource $image,
  int $x1,
  int $y1,
  int $x2,
  int $y2,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imageloadfont(string $file): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagepalettecopy(
  resource $destination,
  resource $source,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagepng(
  resource $image,
  string $filename = "",
  int $quality = -1,
  int $filters = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagepolygon(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $points,
  int $num_points,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagerectangle(
  resource $image,
  int $x1,
  int $y1,
  int $x2,
  int $y2,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagerotate(
  resource $source_image,
  float $angle,
  int $bgd_color,
  int $ignore_transparent = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesavealpha(
  resource $image,
  bool $saveflag,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesetbrush(
  resource $image,
  resource $brush,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesetpixel(
  resource $image,
  int $x,
  int $y,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesetstyle(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $style,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesetthickness(
  resource $image,
  int $thickness,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesettile(
  resource $image,
  resource $tile,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagestring(
  resource $image,
  int $font,
  int $x,
  int $y,
  string $str,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagestringup(
  resource $image,
  int $font,
  int $x,
  int $y,
  string $str,
  int $color,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesx(resource $image): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagesy(resource $image): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagetruecolortopalette(
  resource $image,
  bool $dither,
  int $ncolors,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagettfbbox(
  float $size,
  float $angle,
  string $fontfile,
  string $text,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagettftext(
  resource $image,
  HH\FIXME\MISSING_PARAM_TYPE $size,
  HH\FIXME\MISSING_PARAM_TYPE $angle,
  int $x,
  int $y,
  int $color,
  string $fontfile,
  string $text,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagetypes(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function imagewbmp(
  resource $image,
  string $filename = "",
  int $foreground = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function iptcembed(
  string $iptcdata,
  string $jpeg_file_name,
  int $spool = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function iptcparse(string $iptcblock): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function jpeg2wbmp(
  string $jpegname,
  string $wbmpname,
  int $dest_height,
  int $dest_width,
  int $threshold,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function png2wbmp(
  string $pngname,
  string $wbmpname,
  int $dest_height,
  int $dest_width,
  int $threshold,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function exif_imagetype(string $filename): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function exif_read_data(
  string $filename,
  string $sections = "",
  bool $arrays = false,
  bool $thumbnail = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function read_exif_data(
  string $filename,
  string $sections = "",
  bool $arrays = false,
  bool $thumbnail = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function exif_tagname(int $index): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function exif_thumbnail(
  string $filename,
  inout HH\FIXME\MISSING_PARAM_TYPE $width,
  inout HH\FIXME\MISSING_PARAM_TYPE $height,
  inout HH\FIXME\MISSING_PARAM_TYPE $imagetype,
): HH\FIXME\MISSING_RETURN_TYPE;
