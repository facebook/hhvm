<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
function gd_info();
<<__PHPStdLib>>
function getimagesize(string $filename, inout $imageinfo);
<<__PHPStdLib>>
function image_type_to_extension(int $imagetype, bool $include_dot = true);
<<__PHPStdLib>>
function image_type_to_mime_type(int $imagetype);
<<__PHPStdLib>>
function image2wbmp(resource $image, string $filename = "", int $threshold = -1);
<<__PHPStdLib>>
function imageaffine(resource $image, $affine = varray[], $clip = darray[]);
<<__PHPStdLib>>
function imageaffinematrixconcat($m1, $m2);
<<__PHPStdLib>>
function imageaffinematrixget(int $type, $options = darray[]);
<<__PHPStdLib>>
function imagealphablending(resource $image, bool $blendmode);
<<__PHPStdLib>>
function imageantialias(resource $image, bool $on);
<<__PHPStdLib>>
function imagearc(resource $image, int $cx, int $cy, int $width, int $height, int $start, int $end, int $color);
<<__PHPStdLib>>
function imagechar(resource $image, int $font, int $x, int $y, string $c, int $color);
<<__PHPStdLib>>
function imagecharup(resource $image, int $font, int $x, int $y, string $c, int $color);
<<__PHPStdLib>>
function imagecolorallocate(resource $image, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolorallocatealpha(resource $image, int $red, int $green, int $blue, int $alpha);
<<__PHPStdLib>>
function imagecolorat(resource $image, int $x, int $y);
<<__PHPStdLib>>
function imagecolorclosest(resource $image, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolorclosestalpha(resource $image, int $red, int $green, int $blue, int $alpha);
<<__PHPStdLib>>
function imagecolorclosesthwb(resource $image, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolordeallocate(resource $image, int $color);
<<__PHPStdLib>>
function imagecolorexact(resource $image, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolorexactalpha(resource $image, int $red, int $green, int $blue, int $alpha);
<<__PHPStdLib>>
function imagecolormatch(resource $image1, resource $image2);
<<__PHPStdLib>>
function imagecolorresolve(resource $image, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolorresolvealpha(resource $image, int $red, int $green, int $blue, int $alpha);
<<__PHPStdLib>>
function imagecolorset(resource $image, int $index, int $red, int $green, int $blue);
<<__PHPStdLib>>
function imagecolorsforindex(resource $image, int $index);
<<__PHPStdLib>>
function imagecolorstotal(resource $image);
<<__PHPStdLib>>
function imagecolortransparent(resource $image, int $color = -1);
<<__PHPStdLib>>
function imageconvolution(resource $image, $matrix, float $div, float $offset);
<<__PHPStdLib>>
function imagecopy(resource $dst_im, resource $src_im, int $dst_x, int $dst_y, int $src_x, int $src_y, int $src_w, int $src_h);
<<__PHPStdLib>>
function imagecopymerge(resource $dst_im, resource $src_im, int $dst_x, int $dst_y, int $src_x, int $src_y, int $src_w, int $src_h, int $pct);
<<__PHPStdLib>>
function imagecopymergegray(resource $dst_im, resource $src_im, int $dst_x, int $dst_y, int $src_x, int $src_y, int $src_w, int $src_h, int $pct);
<<__PHPStdLib>>
function imagecopyresampled(resource $dst_im, resource $src_im, int $dst_x, int $dst_y, int $src_x, int $src_y, int $dst_w, int $dst_h, int $src_w, int $src_h);
<<__PHPStdLib>>
function imagecopyresized(resource $dst_im, resource $src_im, int $dst_x, int $dst_y, int $src_x, int $src_y, int $dst_w, int $dst_h, int $src_w, int $src_h);
<<__PHPStdLib>>
function imagecreate(int $width, int $height);
<<__PHPStdLib>>
function imagecreatefromgd2part(string $filename, int $srcx, int $srcy, int $width, int $height);
<<__PHPStdLib>>
function imagecreatefromgd(string $filename);
<<__PHPStdLib>>
function imagecreatefromgd2(string $filename);
<<__PHPStdLib>>
function imagecreatefromgif(string $filename);
<<__PHPStdLib>>
function imagecreatefromjpeg(string $filename);
<<__PHPStdLib>>
function imagecreatefrompng(string $filename);
<<__PHPStdLib>>
function imagecreatefromstring(string $data);
<<__PHPStdLib>>
function imagecreatefromwbmp(string $filename);
<<__PHPStdLib>>
function imagecreatefromxbm(string $filename);
function imagecreatefromxpm(string $filename);
<<__PHPStdLib>>
function imagecreatetruecolor(int $width, int $height);
<<__PHPStdLib>>
function imagecrop(resource $image, $rect);
<<__PHPStdLib>>
function imagecropauto(resource $image, int $mode = -1, float $threshold = 0.5, int $color = -1);
<<__PHPStdLib>>
function imagedashedline(resource $image, int $x1, int $y1, int $x2, int $y2, int $color);
<<__PHPStdLib>>
function imagedestroy(resource $image);
<<__PHPStdLib>>
function imageellipse(resource $image, int $cx, int $cy, int $width, int $height, int $color);
<<__PHPStdLib>>
function imagefill(resource $image, int $x, int $y, int $color);
<<__PHPStdLib>>
function imagefilledarc(resource $image, int $cx, int $cy, int $width, int $height, int $start, int $end, int $color, int $style);
<<__PHPStdLib>>
function imagefilledellipse(resource $image, int $cx, int $cy, int $width, int $height, int $color);
<<__PHPStdLib>>
function imagefilledpolygon(resource $image, $points, int $num_points, int $color);
<<__PHPStdLib>>
function imagefilledrectangle(resource $image, int $x1, int $y1, int $x2, int $y2, int $color);
<<__PHPStdLib>>
function imagefilltoborder(resource $image, int $x, int $y, int $border, int $color);
<<__PHPStdLib>>
function imagefilter(resource $image, int $filtertype, $arg1 = 0, $arg2 = 0, $arg3 = 0, $arg4 = 0);
<<__PHPStdLib>>
function imageflip(resource $image, int $mode = -1);
<<__PHPStdLib>>
function imagefontheight(int $font);
<<__PHPStdLib>>
function imagefontwidth(int $font);
<<__PHPStdLib>>
function imageftbbox(float $size, float $angle, string $font_file, string $text, $extrainfo = null);
<<__PHPStdLib>>
function imagefttext(resource $image, $size, $angle, int $x, int $y, int $col, string $font_file, string $text, $extrainfo = null);
<<__PHPStdLib>>
function imagegammacorrect(resource $image, float $inputgamma, float $outputgamma);
<<__PHPStdLib>>
function imagegd2(resource $image, string $filename = "", int $chunk_size = 0, int $type = 0);
<<__PHPStdLib>>
function imagegd(resource $image, string $filename = "");
<<__PHPStdLib>>
function imagegif(resource $image, string $filename = "");
<<__PHPStdLib>>
function imagegrabscreen();
<<__PHPStdLib>>
function imagegrabwindow($window, $client_area = 0);
<<__PHPStdLib>>
function imageinterlace(resource $image, ?int $interlace = null);
<<__PHPStdLib>>
function imageistruecolor(resource $image);
<<__PHPStdLib>>
function imagejpeg(resource $image, string $filename = "", int $quality = -1);
<<__PHPStdLib>>
function imagelayereffect(resource $image, int $effect);
<<__PHPStdLib>>
function imageline(resource $image, int $x1, int $y1, int $x2, int $y2, int $color);
<<__PHPStdLib>>
function imageloadfont(string $file);
<<__PHPStdLib>>
function imagepalettecopy(resource $destination, resource $source);
<<__PHPStdLib>>
function imagepng(resource $image, string $filename = "", int $quality = -1, int $filters = -1);
<<__PHPStdLib>>
function imagepolygon(resource $image, $points, int $num_points, int $color);
<<__PHPStdLib>>
function imagerectangle(resource $image, int $x1, int $y1, int $x2, int $y2, int $color);
<<__PHPStdLib>>
function imagerotate(resource $source_image, float $angle, int $bgd_color, int $ignore_transparent = 0);
<<__PHPStdLib>>
function imagesavealpha(resource $image, bool $saveflag);
<<__PHPStdLib>>
function imagesetbrush(resource $image, resource $brush);
<<__PHPStdLib>>
function imagesetpixel(resource $image, int $x, int $y, int $color);
<<__PHPStdLib>>
function imagesetstyle(resource $image, $style);
<<__PHPStdLib>>
function imagesetthickness(resource $image, int $thickness);
<<__PHPStdLib>>
function imagesettile(resource $image, resource $tile);
<<__PHPStdLib>>
function imagestring(resource $image, int $font, int $x, int $y, string $str, int $color);
<<__PHPStdLib>>
function imagestringup(resource $image, int $font, int $x, int $y, string $str, int $color);
<<__PHPStdLib>>
function imagesx(resource $image);
<<__PHPStdLib>>
function imagesy(resource $image);
<<__PHPStdLib>>
function imagetruecolortopalette(resource $image, bool $dither, int $ncolors);
<<__PHPStdLib>>
function imagettfbbox(float $size, float $angle, string $fontfile, string $text);
<<__PHPStdLib>>
function imagettftext(resource $image, $size, $angle, int $x, int $y, int $color, string $fontfile, string $text);
<<__PHPStdLib>>
function imagetypes();
<<__PHPStdLib>>
function imagewbmp(resource $image, string $filename = "", int $foreground = -1);

function imagewebp(resource $image, string $filename = "", int $quality = 80);

function imagexbm(resource $image, string $filename = "", int $foreground = -1);
<<__PHPStdLib>>
function iptcembed(string $iptcdata, string $jpeg_file_name, int $spool = 0);
<<__PHPStdLib>>
function iptcparse(string $iptcblock);
<<__PHPStdLib>>
function jpeg2wbmp(string $jpegname, string $wbmpname, int $dest_height, int $dest_width, int $threshold);
<<__PHPStdLib>>
function png2wbmp(string $pngname, string $wbmpname, int $dest_height, int $dest_width, int $threshold);
<<__PHPStdLib>>
function exif_imagetype(string $filename);
<<__PHPStdLib>>
function exif_read_data(string $filename, string $sections = "", bool $arrays = false, bool $thumbnail = false);
<<__PHPStdLib>>
function read_exif_data(string $filename, string $sections = "", bool $arrays = false, bool $thumbnail = false);
<<__PHPStdLib>>
function exif_tagname(int $index);
<<__PHPStdLib>>
function exif_thumbnail(string $filename, inout $width,
                        inout $height, inout $imagetype);
