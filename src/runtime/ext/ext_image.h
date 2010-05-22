/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_IMAGE_H__
#define __EXT_IMAGE_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/zend/zend_php_config.h>
#include <gd.h>

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
  Image() { m_gdImage = NULL;}
  Image(gdImagePtr gdImage) { m_gdImage = gdImage;}
  ~Image();
  gdImagePtr get() { return m_gdImage;}
  void reset() { m_gdImage = NULL;}

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "gd";}
  virtual bool isResource() const { return m_gdImage != NULL;}

private:
  gdImagePtr m_gdImage;
};

Array f_gd_info();
Variant f_getimagesize(CStrRef filename, Variant imageinfo = null);
String f_image_type_to_extension(int imagetype, bool include_dot = true);
String f_image_type_to_mime_type(int imagetype);
bool f_image2wbmp(CObjRef image, CStrRef filename = null_string, int threshold = -1);
bool f_imagealphablending(CObjRef image, bool blendmode);
bool f_imageantialias(CObjRef image, bool on);
bool f_imagearc(CObjRef image, int cx, int cy, int width, int height, int start, int end, int color);
bool f_imagechar(CObjRef image, int font, int x, int y, CStrRef c, int color);
bool f_imagecharup(CObjRef image, int font, int x, int y, CStrRef c, int color);
Variant f_imagecolorallocate(CObjRef image, int red, int green, int blue);
Variant f_imagecolorallocatealpha(CObjRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorat(CObjRef image, int x, int y);
Variant f_imagecolorclosest(CObjRef image, int red, int green, int blue);
Variant f_imagecolorclosestalpha(CObjRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorclosesthwb(CObjRef image, int red, int green, int blue);
bool f_imagecolordeallocate(CObjRef image, int color);
Variant f_imagecolorexact(CObjRef image, int red, int green, int blue);
Variant f_imagecolorexactalpha(CObjRef image, int red, int green, int blue, int alpha);
Variant f_imagecolormatch(CObjRef image1, CObjRef image2);
Variant f_imagecolorresolve(CObjRef image, int red, int green, int blue);
Variant f_imagecolorresolvealpha(CObjRef image, int red, int green, int blue, int alpha);
Variant f_imagecolorset(CObjRef image, int index, int red, int green, int blue);
Variant f_imagecolorsforindex(CObjRef image, int index);
Variant f_imagecolorstotal(CObjRef image);
Variant f_imagecolortransparent(CObjRef image, int color = -1);
bool f_imageconvolution(CObjRef image, CArrRef matrix, double div, double offset);
bool f_imagecopy(CObjRef dst_im, CObjRef src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h);
bool f_imagecopymerge(CObjRef dst_im, CObjRef src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopymergegray(CObjRef dst_im, CObjRef src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct);
bool f_imagecopyresampled(CObjRef dst_im, CObjRef src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h);
bool f_imagecopyresized(CObjRef dst_im, CObjRef src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h);
Variant f_imagecreate(int width, int height);
Variant f_imagecreatefromgd2part(CStrRef filename, int srcx, int srcy, int width, int height);
Variant f_imagecreatefromgd(CStrRef filename);
Variant f_imagecreatefromgd2(CStrRef filename);
Variant f_imagecreatefromgif(CStrRef filename);
Variant f_imagecreatefromjpeg(CStrRef filename);
Variant f_imagecreatefrompng(CStrRef filename);
Variant f_imagecreatefromstring(CStrRef data);
Variant f_imagecreatefromwbmp(CStrRef filename);
Variant f_imagecreatefromxbm(CStrRef filename);
Variant f_imagecreatefromxpm(CStrRef filename);
Variant f_imagecreatetruecolor(int width, int height);
bool f_imagedashedline(CObjRef image, int x1, int y1, int x2, int y2, int color);
bool f_imagedestroy(CObjRef image);
bool f_imageellipse(CObjRef image, int cx, int cy, int width, int height, int color);
bool f_imagefill(CObjRef image, int x, int y, int color);
bool f_imagefilledarc(CObjRef image, int cx, int cy, int width, int height, int start, int end, int color, int style);
bool f_imagefilledellipse(CObjRef image, int cx, int cy, int width, int height, int color);
bool f_imagefilledpolygon(CObjRef image, CArrRef points, int num_points, int color);
bool f_imagefilledrectangle(CObjRef image, int x1, int y1, int x2, int y2, int color);
bool f_imagefilltoborder(CObjRef image, int x, int y, int border, int color);
bool f_imagefilter(CObjRef image, int filtertype, int arg1 = 0, int arg2 = 0, int arg3 = 0, int arg4 = 0);
int f_imagefontheight(int font);
int f_imagefontwidth(int font);
Variant f_imageftbbox(double size, double angle, CStrRef font_file, CStrRef text, CArrRef extrainfo = null);
Variant f_imagefttext(CObjRef image, double size, double angle, int x, int y, int col, CStrRef font_file, CStrRef text, CArrRef extrainfo = null);
bool f_imagegammacorrect(CObjRef image, double inputgamma, double outputgamma);
bool f_imagegd2(CObjRef image, CStrRef filename = null_string, int chunk_size = 0, int type = 0);
bool f_imagegd(CObjRef image, CStrRef filename = null_string);
bool f_imagegif(CObjRef image, CStrRef filename = null_string);
Variant f_imagegrabscreen();
Variant f_imagegrabwindow(int window, int client_area = 0);
Variant f_imageinterlace(CObjRef image, int interlace = 0);
bool f_imageistruecolor(CObjRef image);
bool f_imagejpeg(CObjRef image, CStrRef filename = null_string, int quality = -1);
bool f_imagelayereffect(CObjRef image, int effect);
bool f_imageline(CObjRef image, int x1, int y1, int x2, int y2, int color);
Variant f_imageloadfont(CStrRef file);
void f_imagepalettecopy(CObjRef destination, CObjRef source);
bool f_imagepng(CObjRef image, CStrRef filename = null_string, int quality = -1, int filters = -1);
bool f_imagepolygon(CObjRef image, CArrRef points, int num_points, int color);
Array f_imagepsbbox(CStrRef text, int font, int size, int space = 0, int tightness = 0, double angle = 0.0);
bool f_imagepsencodefont(CObjRef font_index, CStrRef encodingfile);
bool f_imagepsextendfont(int font_index, double extend);
bool f_imagepsfreefont(CObjRef fontindex);
Object f_imagepsloadfont(CStrRef filename);
bool f_imagepsslantfont(CObjRef font_index, double slant);
Array f_imagepstext(CObjRef image, CStrRef text, CObjRef font, int size, int foreground, int background, int x, int y, int space = 0, int tightness = 0, double angle = 0.0, int antialias_steps = 0);
bool f_imagerectangle(CObjRef image, int x1, int y1, int x2, int y2, int color);
Variant f_imagerotate(CObjRef source_image, double angle, int bgd_color, int ignore_transparent = 0);
bool f_imagesavealpha(CObjRef image, bool saveflag);
bool f_imagesetbrush(CObjRef image, CObjRef brush);
bool f_imagesetpixel(CObjRef image, int x, int y, int color);
bool f_imagesetstyle(CObjRef image, CArrRef style);
bool f_imagesetthickness(CObjRef image, int thickness);
bool f_imagesettile(CObjRef image, CObjRef tile);
bool f_imagestring(CObjRef image, int font, int x, int y, CStrRef str, int color);
bool f_imagestringup(CObjRef image, int font, int x, int y, CStrRef str, int color);
Variant f_imagesx(CObjRef image);
Variant f_imagesy(CObjRef image);
Variant f_imagetruecolortopalette(CObjRef image, bool dither, int ncolors);
Variant f_imagettfbbox(double size, double angle, CStrRef fontfile, CStrRef text);
Variant f_imagettftext(CObjRef image, double size, double angle, int x, int y, int color, CStrRef fontfile, CStrRef text);
int f_imagetypes();
bool f_imagewbmp(CObjRef image, CStrRef filename = null_string, int foreground = -1);
bool f_imagexbm(CObjRef image, CStrRef filename = null_string, int foreground = -1);
Variant f_iptcembed(CStrRef iptcdata, CStrRef jpeg_file_name, int spool = 0);
Variant f_iptcparse(CStrRef iptcblock);
bool f_jpeg2wbmp(CStrRef jpegname, CStrRef wbmpname, int dest_height, int dest_width, int threshold);
bool f_png2wbmp(CStrRef pngname, CStrRef wbmpname, int dest_height, int dest_width, int threshold);
Variant f_exif_imagetype(CStrRef filename);
Variant f_exif_read_data(CStrRef filename, CStrRef sections = null_string, bool arrays = false, bool thumbnail = false);
Variant f_read_exif_data(CStrRef filename, CStrRef sections = null_string, bool arrays = false, bool thumbnail = false);
Variant f_exif_tagname(int index);
Variant f_exif_thumbnail(CStrRef filename, Variant width = null, Variant height = null, Variant imagetype = null);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_IMAGE_H__
