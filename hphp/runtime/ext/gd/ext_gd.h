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


#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/zend-php-config.h"

typedef struct gdImageStruct* gdImagePtr;

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
  gdImagePtr get() { return m_gdImage;}
  void reset();

  CLASSNAME_IS("gd")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return m_gdImage == nullptr; }

  DECLARE_RESOURCE_ALLOCATION(Image)
private:
  gdImagePtr m_gdImage;
};

Array HHVM_FUNCTION(gd_info);
Variant HHVM_FUNCTION(getimagesize,
  const String& filename, VRefParam imageinfo = uninit_null());
String HHVM_FUNCTION(image_type_to_extension,
  int64_t imagetype, bool include_dot = true);
String HHVM_FUNCTION(image_type_to_mime_type, int64_t imagetype);
#ifdef HAVE_GD_WBMP
bool HHVM_FUNCTION(image2wbmp, const Resource& image,
  const String& filename = null_string, int64_t threshold = -1);
#endif
bool HHVM_FUNCTION(imagealphablending, const Resource& image, bool blendmode);
bool HHVM_FUNCTION(imageantialias, const Resource& image, bool on);
bool HHVM_FUNCTION(imagearc, const Resource& image,
  int64_t cx, int64_t cy, int64_t width, int64_t height,
  int64_t start, int64_t end, int64_t color);
bool HHVM_FUNCTION(imagechar, const Resource& image,
  int64_t font, int64_t x, int64_t y, const String& c, int64_t color);
bool HHVM_FUNCTION(imagecharup, const Resource& image,
  int64_t font, int64_t x, int64_t y, const String& c, int64_t color);
Variant HHVM_FUNCTION(imagecolorallocate, const Resource& image,
  int64_t red, int64_t green, int64_t blue);
Variant HHVM_FUNCTION(imagecolorallocatealpha,
  const Resource& image, int64_t red, int64_t green,
  int64_t blue, int64_t alpha);
Variant HHVM_FUNCTION(imagecolorat, const Resource& image,
  int64_t x, int64_t y);
Variant HHVM_FUNCTION(imagecolorclosest, const Resource& image,
  int64_t red, int64_t green, int64_t blue);
Variant HHVM_FUNCTION(imagecolorclosestalpha, const Resource& image,
  int64_t red, int64_t green, int64_t blue, int64_t alpha);
Variant HHVM_FUNCTION(imagecolorclosesthwb,
  const Resource& image, int64_t red, int64_t green, int64_t blue);
bool HHVM_FUNCTION(imagecolordeallocate, const Resource& image, int64_t color);
Variant HHVM_FUNCTION(imagecolorexact,
  const Resource& image, int64_t red, int64_t green, int64_t blue);
Variant HHVM_FUNCTION(imagecolorexactalpha, const Resource& image,
 int64_t red, int64_t green, int64_t blue, int64_t alpha);
Variant HHVM_FUNCTION(imagecolormatch, const Resource& image1,
                                       const Resource& image2);
Variant HHVM_FUNCTION(imagecolorresolve,
  const Resource& image, int64_t red, int64_t green, int64_t blue);
Variant HHVM_FUNCTION(imagecolorresolvealpha,  const Resource& image,
  int64_t red, int64_t green, int64_t blue, int64_t alpha);
Variant HHVM_FUNCTION(imagecolorset, const Resource& image,
  int64_t index, int64_t red, int64_t green, int64_t blue);
Variant HHVM_FUNCTION(imagecolorsforindex,
  const Resource& image, int64_t index);
Variant HHVM_FUNCTION(imagecolorstotal,
  const Resource& image);
Variant HHVM_FUNCTION(imagecolortransparent,
  const Resource& image, int64_t color = -1);
bool HHVM_FUNCTION(imageconvolution,
  const Resource& image, const Array& matrix, double div, double offset);
bool HHVM_FUNCTION(imagecopy, const Resource& dst_im, const Resource& src_im,
  int64_t dst_x, int64_t dst_y,
  int64_t src_x, int64_t src_y, int64_t src_w, int64_t src_h);
bool HHVM_FUNCTION(imagecopymerge, const Resource& dst_im,
  const Resource& src_im, int64_t dst_x, int64_t dst_y,
  int64_t src_x, int64_t src_y, int64_t src_w, int64_t src_h, int64_t pct);
bool HHVM_FUNCTION(imagecopymergegray, const Resource& dst_im,
  const Resource& src_im, int64_t dst_x, int64_t dst_y,
  int64_t src_x, int64_t src_y, int64_t src_w, int64_t src_h, int64_t pct);
bool HHVM_FUNCTION(imagecopyresampled,  const Resource& dst_im,
  const Resource& src_im, int64_t dst_x, int64_t dst_y,
  int64_t src_x, int64_t src_y, int64_t dst_w, int64_t dst_h,
  int64_t src_w, int64_t src_h);
bool HHVM_FUNCTION(imagecopyresized,
  const Resource& dst_im, const Resource& src_im,
  int64_t dst_x, int64_t dst_y, int64_t src_x, int64_t src_y,
  int64_t dst_w, int64_t dst_h, int64_t src_w, int64_t src_h);
Variant HHVM_FUNCTION(imagecreate, int64_t width, int64_t height);
Variant HHVM_FUNCTION(imagecreatefromgd2part, const String& filename,
  int64_t srcx, int64_t srcy, int64_t width, int64_t height);
Variant HHVM_FUNCTION(imagecreatefromgd, const String& filename);
Variant HHVM_FUNCTION(imagecreatefromgd2, const String& filename);
Variant HHVM_FUNCTION(imagecreatefromgif, const String& filename);
#ifdef HAVE_GD_JPG
Variant HHVM_FUNCTION(imagecreatefromjpeg, const String& filename);
#endif
#ifdef HAVE_GD_PNG
Variant HHVM_FUNCTION(imagecreatefrompng, const String& filename);
#endif
#ifdef HAVE_LIBVPX
Variant HHVM_FUNCTION(imagecreatefromwebp, const String& filename);
#endif
#ifdef HAVE_LIBGD15
Variant HHVM_FUNCTION(imagecreatefromstring, const String& data);
#endif
#ifdef HAVE_GD_WBMP
Variant HHVM_FUNCTION(imagecreatefromwbmp, const String& filename);
#endif
#ifdef HAVE_GD_XBM
Variant HHVM_FUNCTION(imagecreatefromxbm, const String& filename);
#endif
#if defined(HAVE_GD_XPM) && defined(HAVE_GD_BUNDLED)
Variant HHVM_FUNCTION(imagecreatefromxpm, const String& filename);
#endif
Variant HHVM_FUNCTION(imagecreatetruecolor, int64_t width, int64_t height);
bool HHVM_FUNCTION(imagedashedline, const Resource& image,
  int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
bool HHVM_FUNCTION(imagedestroy, const Resource& image);
bool HHVM_FUNCTION(imageellipse,  const Resource& image,
 int64_t cx, int64_t cy, int64_t width, int64_t height, int64_t color);
bool HHVM_FUNCTION(imagefill, const Resource& image,
 int64_t x, int64_t y, int64_t color);
bool HHVM_FUNCTION(imagefilledarc, const Resource& image,
  int64_t cx, int64_t cy, int64_t width, int64_t height,
  int64_t start, int64_t end, int64_t color, int64_t style);
bool HHVM_FUNCTION(imagefilledellipse, const Resource& image,
  int64_t cx, int64_t cy, int64_t width, int64_t height, int64_t color);
bool HHVM_FUNCTION(imagefilledpolygon, const Resource& image,
  const Array& points, int64_t num_points, int64_t color);
bool HHVM_FUNCTION(imagefilledrectangle, const Resource& image,
  int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
bool HHVM_FUNCTION(imagefilltoborder, const Resource& image,
  int64_t x, int64_t y, int64_t border, int64_t color);
bool HHVM_FUNCTION(imagefilter, const Resource& image,
  int64_t filtertype,
  const Variant& arg1 = 0, const Variant& arg2 = 0,
  const Variant& arg3 = 0, const Variant& arg4 = 0);
int64_t HHVM_FUNCTION(imagefontheight, int64_t font);
int64_t HHVM_FUNCTION(imagefontwidth, int64_t font);
#if defined(ENABLE_GD_TTF) && HAVE_LIBGD20 && \
    HAVE_LIBFREETYPE && HAVE_GD_STRINGFTEX
Variant HHVM_FUNCTION(imageftbbox,
  double size, double angle, const String& font_file, const String& text,
  const Array& extrainfo = Array());
#endif
bool HHVM_FUNCTION(imagegammacorrect, const Resource& image,
  double inputgamma, double outputgamma);
bool HHVM_FUNCTION(imagegd2, const Resource& image,
  const String& filename = null_string,
  int64_t chunk_size = 0, int64_t type = 0);
bool HHVM_FUNCTION(imagegd, const Resource& image,
                            const String& filename = null_string);
bool HHVM_FUNCTION(imagegif, const Resource& image,
                             const String& filename = null_string);
Variant HHVM_FUNCTION(imageinterlace, const Resource& image,
  int64_t interlace = 0);
bool HHVM_FUNCTION(imageistruecolor, const Resource& image);
#ifdef HAVE_GD_JPG
bool HHVM_FUNCTION(imagejpeg, const Resource& image,
  const String& filename = null_string, int64_t quality = -1);
#endif
bool HHVM_FUNCTION(imagelayereffect, const Resource& image, int64_t effect);
bool HHVM_FUNCTION(imageline, const Resource& image,
  int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
Variant HHVM_FUNCTION(imageloadfont, const String& file);
#ifdef HAVE_GD_PNG
bool HHVM_FUNCTION(imagepng,  const Resource& image,
  const String& filename = null_string, int64_t quality = -1,
  int64_t filters = -1);
#endif
#ifdef HAVE_LIBVPX
bool HHVM_FUNCTION(imagewebp,  const Resource& image,
  const String& filename = null_string, int64_t quality = 80);
#endif
bool HHVM_FUNCTION(imagepolygon, const Resource& image,
  const Array& points, int64_t num_points, int64_t color);
bool HHVM_FUNCTION(imagerectangle, const Resource& image,
  int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t color);
Variant HHVM_FUNCTION(imagerotate,
  const Resource& source_image, double angle, int64_t bgd_color,
  int64_t ignore_transparent = 0);
bool HHVM_FUNCTION(imagesavealpha, const Resource& image, bool saveflag);
bool HHVM_FUNCTION(imagesetbrush, const Resource& image,
  const Resource& brush);
bool HHVM_FUNCTION(imagesetpixel, const Resource& image,
  int64_t x, int64_t y, int64_t color);
bool HHVM_FUNCTION(imagesetstyle, const Resource& image, const Array& style);
bool HHVM_FUNCTION(imagesetthickness, const Resource& image,
  int64_t thickness);
#if HAVE_GD_IMAGESETTILE
bool HHVM_FUNCTION(imagesettile, const Resource& image, const Resource& tile);
#endif
bool HHVM_FUNCTION(imagestring,  const Resource& image,
  int64_t font, int64_t x, int64_t y,
  const String& str, int64_t color);
bool HHVM_FUNCTION(imagestringup,  const Resource& image,
  int64_t font, int64_t x, int64_t y, const String& str, int64_t color);
Variant HHVM_FUNCTION(imagesx, const Resource& image);
Variant HHVM_FUNCTION(imagesy, const Resource& image);
Variant HHVM_FUNCTION(imagetruecolortopalette,
  const Resource& image, bool dither, int64_t ncolors);
#ifdef ENABLE_GD_TTF
Variant HHVM_FUNCTION(imagettfbbox,
  double size, double angle, const String& fontfile, const String& text);
#endif
int64_t HHVM_FUNCTION(imagetypes);
bool HHVM_FUNCTION(imagewbmp, const Resource& image,
  const String& filename = null_string, int64_t foreground = -1);
Variant HHVM_FUNCTION(iptcembed,
  const String& iptcdata, const String& jpeg_file_name, int64_t spool = 0);
Variant HHVM_FUNCTION(iptcparse, const String& iptcblock);
bool HHVM_FUNCTION(jpeg2wbmp,
  const String& jpegname, const String& wbmpname, int64_t dest_height,
  int64_t dest_width, int64_t threshold);
bool HHVM_FUNCTION(png2wbmp,
  const String& pngname, const String& wbmpname, int64_t dest_height,
  int64_t dest_width, int64_t threshold);
Variant HHVM_FUNCTION(exif_imagetype, const String& filename);
Variant HHVM_FUNCTION(exif_read_data,
  const String& filename, const String& sections = empty_string_ref,
  bool arrays = false, bool thumbnail = false);
Variant HHVM_FUNCTION(read_exif_data,
  const String& filename, const String& sections = null_string,
  bool arrays = false, bool thumbnail = false);
Variant HHVM_FUNCTION(exif_tagname, int64_t index);
Variant HHVM_FUNCTION(exif_thumbnail,
  const String& filename, VRefParam width = uninit_null(),
  VRefParam height = uninit_null(), VRefParam imagetype = uninit_null());
Variant HHVM_FUNCTION(imagepalettecopy,
  const Resource& dest, const Resource& src);
///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_IMAGE_H_
