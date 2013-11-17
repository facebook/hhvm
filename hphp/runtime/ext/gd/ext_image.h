/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_IMAGE_H_
#define incl_HPHP_EXT_IMAGE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/zend-php-config.h"
#include "hphp/runtime/ext/gd/libgd/gd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct gfxinfo {
  unsigned int width;
  unsigned int height;
  unsigned int bits;
  unsigned int channels;
};

class Image : public SweepableResourceData {
public:
  Image() : m_gdImage(nullptr) {}
  explicit Image(gdImagePtr gdImage) : m_gdImage(gdImage) {}
  ~Image();
  void sweep() FOLLY_OVERRIDE;
  gdImagePtr get() { return m_gdImage;}
  void reset() { m_gdImage = nullptr;}

  CLASSNAME_IS("gd")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }
  virtual bool isInvalid() const { return m_gdImage == nullptr; }

private:
  gdImagePtr m_gdImage;
};

Array f_gd_info();
Variant f_getimagesize(
  const String& filename, VRefParam imageinfo = uninit_null());
String f_image_type_to_extension(int imagetype, bool include_dot = true);
String f_image_type_to_mime_type(int imagetype);
bool f_image2wbmp(
  CResRef image, const String& filename = null_string, int threshold = -1);
bool f_imagealphablending(CResRef image, bool blendmode);
bool f_imageantialias(CResRef image, bool on);
bool f_imagearc(
  CResRef image, int cx, int cy, int width, int height, int start, int end,
  int color);
bool f_imagechar(
  CResRef image, int font, int x, int y, const String& c, int color);
bool f_imagecharup(
  CResRef image, int font, int x, int y, const String& c, int color);
Variant f_imagecolorallocate(CResRef image, int red, int green, int blue);
Variant f_imagecolorallocatealpha(
  CResRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorat(CResRef image, int x, int y);
Variant f_imagecolorclosest(CResRef image, int red, int green, int blue);
Variant f_imagecolorclosestalpha(
  CResRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorclosesthwb(CResRef image, int red, int green, int blue);
bool f_imagecolordeallocate(CResRef image, int color);
Variant f_imagecolorexact(CResRef image, int red, int green, int blue);
Variant f_imagecolorexactalpha(
  CResRef image, int red, int green, int blue, int alpha);
Variant f_imagecolormatch(CResRef image1, CResRef image2);
Variant f_imagecolorresolve(CResRef image, int red, int green, int blue);
Variant f_imagecolorresolvealpha(
  CResRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorset(CResRef image, int index, int red, int green, int blue);
Variant f_imagecolorsforindex(CResRef image, int index);
Variant f_imagecolorstotal(CResRef image);
Variant f_imagecolortransparent(CResRef image, int color = -1);
bool f_imageconvolution(
  CResRef image, CArrRef matrix, double div, double offset);
bool f_imagecopy(
  CResRef dst_im, CResRef src_im, int dst_x, int dst_y, int src_x, int src_y,
  int src_w, int src_h);
bool f_imagecopymerge(CResRef dst_im, CResRef src_im, int dst_x, int dst_y,
                      int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopymergegray(CResRef dst_im, CResRef src_im, int dst_x, int dst_y,
                          int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopyresampled(
  CResRef dst_im, CResRef src_im, int dst_x, int dst_y,
  int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h);
bool f_imagecopyresized(
  CResRef dst_im, CResRef src_im, int dst_x, int dst_y, int src_x, int src_y,
  int dst_w, int dst_h, int src_w, int src_h);
Variant f_imagecreate(int width, int height);
Variant f_imagecreatefromgd2part(
  const String& filename, int srcx, int srcy, int width, int height);
Variant f_imagecreatefromgd(const String& filename);
Variant f_imagecreatefromgd2(const String& filename);
Variant f_imagecreatefromgif(const String& filename);
Variant f_imagecreatefromjpeg(const String& filename);
Variant f_imagecreatefrompng(const String& filename);
Variant f_imagecreatefromstring(const String& data);
Variant f_imagecreatefromwbmp(const String& filename);
Variant f_imagecreatefromxbm(const String& filename);
Variant f_imagecreatefromxpm(const String& filename);
Variant f_imagecreatetruecolor(int width, int height);
bool f_imagedashedline(
  CResRef image, int x1, int y1, int x2, int y2, int color);
bool f_imagedestroy(CResRef image);
bool f_imageellipse(
  CResRef image, int cx, int cy, int width, int height, int color);
bool f_imagefill(CResRef image, int x, int y, int color);
bool f_imagefilledarc(
  CResRef image, int cx, int cy, int width, int height, int start, int end,
  int color, int style);
bool f_imagefilledellipse(
  CResRef image, int cx, int cy, int width, int height, int color);
bool f_imagefilledpolygon(
  CResRef image, CArrRef points, int num_points, int color);
bool f_imagefilledrectangle(
  CResRef image, int x1, int y1, int x2, int y2, int color);
bool f_imagefilltoborder(CResRef image, int x, int y, int border, int color);
bool f_imagefilter(
  CResRef image, int filtertype, int arg1 = 0, int arg2 = 0, int arg3 = 0,
  int arg4 = 0);
int64_t f_imagefontheight(int font);
int64_t f_imagefontwidth(int font);
Variant f_imageftbbox(
  double size, double angle, const String& font_file, const String& text,
  CArrRef extrainfo = Array());
Variant f_imagefttext(
  CResRef image, double size, double angle, int x, int y, int col,
  const String& font_file, const String& text, CArrRef extrainfo = Array());
bool f_imagegammacorrect(CResRef image, double inputgamma, double outputgamma);
bool f_imagegd2(
  CResRef image, const String& filename = null_string, int chunk_size = 0,
  int type = 0);
bool f_imagegd(CResRef image, const String& filename = null_string);
bool f_imagegif(CResRef image, const String& filename = null_string);
Variant f_imagegrabscreen();
Variant f_imagegrabwindow(int window, int client_area = 0);
Variant f_imageinterlace(CResRef image, int interlace = 0);
bool f_imageistruecolor(CResRef image);
bool f_imagejpeg(
  CResRef image, const String& filename = null_string, int quality = -1);
bool f_imagelayereffect(CResRef image, int effect);
bool f_imageline(CResRef image, int x1, int y1, int x2, int y2, int color);
Variant f_imageloadfont(const String& file);
void f_imagepalettecopy(CResRef destination, CResRef source);
bool f_imagepng(
  CResRef image, const String& filename = null_string, int quality = -1,
  int filters = -1);
bool f_imagepolygon(CResRef image, CArrRef points, int num_points, int color);
Array f_imagepsbbox(
  const String& text, int font, int size, int space = 0, int tightness = 0,
  double angle = 0.0);
bool f_imagepsencodefont(CResRef font_index, const String& encodingfile);
bool f_imagepsextendfont(int font_index, double extend);
bool f_imagepsfreefont(CResRef fontindex);
Resource f_imagepsloadfont(const String& filename);
bool f_imagepsslantfont(CResRef font_index, double slant);
Array f_imagepstext(
  CResRef image, const String& text, CResRef font, int size, int foreground,
  int background, int x, int y, int space = 0, int tightness = 0,
  double angle = 0.0, int antialias_steps = 0);
bool f_imagerectangle(CResRef image, int x1, int y1, int x2, int y2, int color);
Variant f_imagerotate(
  CResRef source_image, double angle, int bgd_color,
  int ignore_transparent = 0);
bool f_imagesavealpha(CResRef image, bool saveflag);
bool f_imagesetbrush(CResRef image, CResRef brush);
bool f_imagesetpixel(CResRef image, int x, int y, int color);
bool f_imagesetstyle(CResRef image, CArrRef style);
bool f_imagesetthickness(CResRef image, int thickness);
bool f_imagesettile(CResRef image, CResRef tile);
bool f_imagestring(
  CResRef image, int font, int x, int y, const String& str, int color);
bool f_imagestringup(
  CResRef image, int font, int x, int y, const String& str, int color);
Variant f_imagesx(CResRef image);
Variant f_imagesy(CResRef image);
Variant f_imagetruecolortopalette(CResRef image, bool dither, int ncolors);
Variant f_imagettfbbox(
  double size, double angle, const String& fontfile, const String& text);
Variant f_imagettftext(
  CResRef image, double size, double angle, int x, int y, int color,
  const String& fontfile, const String& text);
int64_t f_imagetypes();
bool f_imagewbmp(
  CResRef image, const String& filename = null_string, int foreground = -1);
bool f_imagexbm(
  CResRef image, const String& filename = null_string, int foreground = -1);
Variant f_iptcembed(
  const String& iptcdata, const String& jpeg_file_name, int spool = 0);
Variant f_iptcparse(const String& iptcblock);
bool f_jpeg2wbmp(
  const String& jpegname, const String& wbmpname, int dest_height,
  int dest_width, int threshold);
bool f_png2wbmp(
  const String& pngname, const String& wbmpname, int dest_height,
  int dest_width, int threshold);
Variant f_exif_imagetype(const String& filename);
Variant f_exif_read_data(
  const String& filename, const String& sections = null_string,
  bool arrays = false, bool thumbnail = false);
Variant f_read_exif_data(
  const String& filename, const String& sections = null_string,
  bool arrays = false, bool thumbnail = false);
Variant f_exif_tagname(int index);
Variant f_exif_thumbnail(
  const String& filename, VRefParam width = uninit_null(),
  VRefParam height = uninit_null(), VRefParam imagetype = uninit_null());

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_IMAGE_H_
