/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
  const Resource& image, const String& filename = null_string, int threshold = -1);
bool f_imagealphablending(const Resource& image, bool blendmode);
bool f_imageantialias(const Resource& image, bool on);
bool f_imagearc(
  const Resource& image, int cx, int cy, int width, int height, int start, int end,
  int color);
bool f_imagechar(
  const Resource& image, int font, int x, int y, const String& c, int color);
bool f_imagecharup(
  const Resource& image, int font, int x, int y, const String& c, int color);
Variant f_imagecolorallocate(const Resource& image, int red, int green, int blue);
Variant f_imagecolorallocatealpha(
  const Resource& image, int red, int green, int blue, int alpha);
Variant f_imagecolorat(const Resource& image, int x, int y);
Variant f_imagecolorclosest(const Resource& image, int red, int green, int blue);
Variant f_imagecolorclosestalpha(
  const Resource& image, int red, int green, int blue, int alpha);
Variant f_imagecolorclosesthwb(const Resource& image, int red, int green, int blue);
bool f_imagecolordeallocate(const Resource& image, int color);
Variant f_imagecolorexact(const Resource& image, int red, int green, int blue);
Variant f_imagecolorexactalpha(
  const Resource& image, int red, int green, int blue, int alpha);
Variant f_imagecolormatch(const Resource& image1, const Resource& image2);
Variant f_imagecolorresolve(const Resource& image, int red, int green, int blue);
Variant f_imagecolorresolvealpha(
  const Resource& image, int red, int green, int blue, int alpha);
Variant f_imagecolorset(const Resource& image, int index, int red, int green, int blue);
Variant f_imagecolorsforindex(const Resource& image, int index);
Variant f_imagecolorstotal(const Resource& image);
Variant f_imagecolortransparent(const Resource& image, int color = -1);
bool f_imageconvolution(
  const Resource& image, const Array& matrix, double div, double offset);
bool f_imagecopy(
  const Resource& dst_im, const Resource& src_im, int dst_x, int dst_y, int src_x, int src_y,
  int src_w, int src_h);
bool f_imagecopymerge(const Resource& dst_im, const Resource& src_im, int dst_x, int dst_y,
                      int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopymergegray(const Resource& dst_im, const Resource& src_im, int dst_x, int dst_y,
                          int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopyresampled(
  const Resource& dst_im, const Resource& src_im, int dst_x, int dst_y,
  int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h);
bool f_imagecopyresized(
  const Resource& dst_im, const Resource& src_im, int dst_x, int dst_y, int src_x, int src_y,
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
  const Resource& image, int x1, int y1, int x2, int y2, int color);
bool f_imagedestroy(const Resource& image);
bool f_imageellipse(
  const Resource& image, int cx, int cy, int width, int height, int color);
bool f_imagefill(const Resource& image, int x, int y, int color);
bool f_imagefilledarc(
  const Resource& image, int cx, int cy, int width, int height, int start, int end,
  int color, int style);
bool f_imagefilledellipse(
  const Resource& image, int cx, int cy, int width, int height, int color);
bool f_imagefilledpolygon(
  const Resource& image, const Array& points, int num_points, int color);
bool f_imagefilledrectangle(
  const Resource& image, int x1, int y1, int x2, int y2, int color);
bool f_imagefilltoborder(const Resource& image, int x, int y, int border, int color);
bool f_imagefilter(
  const Resource& image, int filtertype, int arg1 = 0, int arg2 = 0, int arg3 = 0,
  int arg4 = 0);
int64_t f_imagefontheight(int font);
int64_t f_imagefontwidth(int font);
Variant f_imageftbbox(
  double size, double angle, const String& font_file, const String& text,
  const Array& extrainfo = Array());
Variant f_imagefttext(
  const Resource& image, double size, double angle, int x, int y, int col,
  const String& font_file, const String& text, const Array& extrainfo = Array());
bool f_imagegammacorrect(const Resource& image, double inputgamma, double outputgamma);
bool f_imagegd2(
  const Resource& image, const String& filename = null_string, int chunk_size = 0,
  int type = 0);
bool f_imagegd(const Resource& image, const String& filename = null_string);
bool f_imagegif(const Resource& image, const String& filename = null_string);
Variant f_imagegrabscreen();
Variant f_imagegrabwindow(int window, int client_area = 0);
Variant f_imageinterlace(const Resource& image, int interlace = 0);
bool f_imageistruecolor(const Resource& image);
bool f_imagejpeg(
  const Resource& image, const String& filename = null_string, int quality = -1);
bool f_imagelayereffect(const Resource& image, int effect);
bool f_imageline(const Resource& image, int x1, int y1, int x2, int y2, int color);
Variant f_imageloadfont(const String& file);
void f_imagepalettecopy(const Resource& destination, const Resource& source);
bool f_imagepng(
  const Resource& image, const String& filename = null_string, int quality = -1,
  int filters = -1);
bool f_imagepolygon(const Resource& image, const Array& points, int num_points, int color);
Array f_imagepsbbox(
  const String& text, int font, int size, int space = 0, int tightness = 0,
  double angle = 0.0);
bool f_imagepsencodefont(const Resource& font_index, const String& encodingfile);
bool f_imagepsextendfont(int font_index, double extend);
bool f_imagepsfreefont(const Resource& fontindex);
Resource f_imagepsloadfont(const String& filename);
bool f_imagepsslantfont(const Resource& font_index, double slant);
Array f_imagepstext(
  const Resource& image, const String& text, const Resource& font, int size, int foreground,
  int background, int x, int y, int space = 0, int tightness = 0,
  double angle = 0.0, int antialias_steps = 0);
bool f_imagerectangle(const Resource& image, int x1, int y1, int x2, int y2, int color);
Variant f_imagerotate(
  const Resource& source_image, double angle, int bgd_color,
  int ignore_transparent = 0);
bool f_imagesavealpha(const Resource& image, bool saveflag);
bool f_imagesetbrush(const Resource& image, const Resource& brush);
bool f_imagesetpixel(const Resource& image, int x, int y, int color);
bool f_imagesetstyle(const Resource& image, const Array& style);
bool f_imagesetthickness(const Resource& image, int thickness);
bool f_imagesettile(const Resource& image, const Resource& tile);
bool f_imagestring(
  const Resource& image, int font, int x, int y, const String& str, int color);
bool f_imagestringup(
  const Resource& image, int font, int x, int y, const String& str, int color);
Variant f_imagesx(const Resource& image);
Variant f_imagesy(const Resource& image);
Variant f_imagetruecolortopalette(const Resource& image, bool dither, int ncolors);
Variant f_imagettfbbox(
  double size, double angle, const String& fontfile, const String& text);
Variant f_imagettftext(
  const Resource& image, double size, double angle, int x, int y, int color,
  const String& fontfile, const String& text);
int64_t f_imagetypes();
bool f_imagewbmp(
  const Resource& image, const String& filename = null_string, int foreground = -1);
bool f_imagexbm(
  const Resource& image, const String& filename = null_string, int foreground = -1);
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
