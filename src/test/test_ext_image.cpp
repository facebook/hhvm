/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_image.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtImage::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_gd_info);
  RUN_TEST(test_getimagesize);
  RUN_TEST(test_image_type_to_extension);
  RUN_TEST(test_image_type_to_mime_type);
  RUN_TEST(test_image2wbmp);
  RUN_TEST(test_imagealphablending);
  RUN_TEST(test_imageantialias);
  RUN_TEST(test_imagearc);
  RUN_TEST(test_imagechar);
  RUN_TEST(test_imagecharup);
  RUN_TEST(test_imagecolorallocate);
  RUN_TEST(test_imagecolorallocatealpha);
  RUN_TEST(test_imagecolorat);
  RUN_TEST(test_imagecolorclosest);
  RUN_TEST(test_imagecolorclosestalpha);
  RUN_TEST(test_imagecolorclosesthwb);
  RUN_TEST(test_imagecolordeallocate);
  RUN_TEST(test_imagecolorexact);
  RUN_TEST(test_imagecolorexactalpha);
  RUN_TEST(test_imagecolormatch);
  RUN_TEST(test_imagecolorresolve);
  RUN_TEST(test_imagecolorresolvealpha);
  RUN_TEST(test_imagecolorset);
  RUN_TEST(test_imagecolorsforindex);
  RUN_TEST(test_imagecolorstotal);
  RUN_TEST(test_imagecolortransparent);
  RUN_TEST(test_imageconvolution);
  RUN_TEST(test_imagecopy);
  RUN_TEST(test_imagecopymerge);
  RUN_TEST(test_imagecopymergegray);
  RUN_TEST(test_imagecopyresampled);
  RUN_TEST(test_imagecopyresized);
  RUN_TEST(test_imagecreate);
  RUN_TEST(test_imagecreatefromgd2part);
  RUN_TEST(test_imagecreatefromgd);
  RUN_TEST(test_imagecreatefromgd2);
  RUN_TEST(test_imagecreatefromgif);
  RUN_TEST(test_imagecreatefromjpeg);
  RUN_TEST(test_imagecreatefrompng);
  RUN_TEST(test_imagecreatefromstring);
  RUN_TEST(test_imagecreatefromwbmp);
  RUN_TEST(test_imagecreatefromxbm);
  RUN_TEST(test_imagecreatefromxpm);
  RUN_TEST(test_imagecreatetruecolor);
  RUN_TEST(test_imagedashedline);
  RUN_TEST(test_imagedestroy);
  RUN_TEST(test_imageellipse);
  RUN_TEST(test_imagefill);
  RUN_TEST(test_imagefilledarc);
  RUN_TEST(test_imagefilledellipse);
  RUN_TEST(test_imagefilledpolygon);
  RUN_TEST(test_imagefilledrectangle);
  RUN_TEST(test_imagefilltoborder);
  RUN_TEST(test_imagefilter);
  RUN_TEST(test_imagefontheight);
  RUN_TEST(test_imagefontwidth);
  RUN_TEST(test_imageftbbox);
  RUN_TEST(test_imagefttext);
  RUN_TEST(test_imagegammacorrect);
  RUN_TEST(test_imagegd2);
  RUN_TEST(test_imagegd);
  RUN_TEST(test_imagegif);
  RUN_TEST(test_imagegrabscreen);
  RUN_TEST(test_imagegrabwindow);
  RUN_TEST(test_imageinterlace);
  RUN_TEST(test_imageistruecolor);
  RUN_TEST(test_imagejpeg);
  RUN_TEST(test_imagelayereffect);
  RUN_TEST(test_imageline);
  RUN_TEST(test_imageloadfont);
  RUN_TEST(test_imagepalettecopy);
  RUN_TEST(test_imagepng);
  RUN_TEST(test_imagepolygon);
  RUN_TEST(test_imagepsbbox);
  RUN_TEST(test_imagepsencodefont);
  RUN_TEST(test_imagepsextendfont);
  RUN_TEST(test_imagepsfreefont);
  RUN_TEST(test_imagepsloadfont);
  RUN_TEST(test_imagepsslantfont);
  RUN_TEST(test_imagepstext);
  RUN_TEST(test_imagerectangle);
  RUN_TEST(test_imagerotate);
  RUN_TEST(test_imagesavealpha);
  RUN_TEST(test_imagesetbrush);
  RUN_TEST(test_imagesetpixel);
  RUN_TEST(test_imagesetstyle);
  RUN_TEST(test_imagesetthickness);
  RUN_TEST(test_imagesettile);
  RUN_TEST(test_imagestring);
  RUN_TEST(test_imagestringup);
  RUN_TEST(test_imagesx);
  RUN_TEST(test_imagesy);
  RUN_TEST(test_imagetruecolortopalette);
  RUN_TEST(test_imagettfbbox);
  RUN_TEST(test_imagettftext);
  RUN_TEST(test_imagetypes);
  RUN_TEST(test_imagewbmp);
  RUN_TEST(test_imagexbm);
  RUN_TEST(test_iptcembed);
  RUN_TEST(test_iptcparse);
  RUN_TEST(test_jpeg2wbmp);
  RUN_TEST(test_png2wbmp);
  RUN_TEST(test_exif_imagetype);
  RUN_TEST(test_exif_read_data);
  RUN_TEST(test_read_exif_data);
  RUN_TEST(test_exif_tagname);
  RUN_TEST(test_exif_thumbnail);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtImage::test_gd_info() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_getimagesize() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_image_type_to_extension() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_image_type_to_mime_type() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_image2wbmp() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagealphablending() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageantialias() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagearc() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagechar() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecharup() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorallocate() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorallocatealpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorat() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorclosest() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorclosestalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorclosesthwb() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolordeallocate() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorexact() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorexactalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolormatch() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorresolve() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorresolvealpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorset() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorsforindex() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolorstotal() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecolortransparent() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageconvolution() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecopy() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecopymerge() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecopymergegray() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecopyresampled() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecopyresized() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreate() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromgd2part() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromgd() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromgd2() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromgif() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromjpeg() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefrompng() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromwbmp() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromxbm() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatefromxpm() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagecreatetruecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagedashedline() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagedestroy() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageellipse() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefill() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilledarc() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilledellipse() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilledpolygon() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilledrectangle() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilltoborder() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefilter() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefontheight() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefontwidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageftbbox() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagefttext() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegammacorrect() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegd2() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegd() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegif() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegrabscreen() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagegrabwindow() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageinterlace() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageistruecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagejpeg() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagelayereffect() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageline() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imageloadfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepalettecopy() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepng() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepolygon() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsbbox() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsencodefont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsextendfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsfreefont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsloadfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepsslantfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagepstext() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagerectangle() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagerotate() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesavealpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesetbrush() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesetpixel() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesetstyle() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesetthickness() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesettile() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagestring() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagestringup() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesx() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagesy() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagetruecolortopalette() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagettfbbox() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagettftext() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagetypes() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagewbmp() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_imagexbm() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_iptcembed() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_iptcparse() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_jpeg2wbmp() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_png2wbmp() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_exif_imagetype() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_exif_read_data() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_read_exif_data() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_exif_tagname() {
  //VCB("<?php ");
  return true;
}

bool TestExtImage::test_exif_thumbnail() {
  //VCB("<?php ");
  return true;
}
