/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

Value* fh_image_type_to_mime_type(Value* _rv, int imagetype) asm("_ZN4HPHP25f_image_type_to_mime_typeEi");

Value* fh_image_type_to_extension(Value* _rv, int imagetype, bool include_dot) asm("_ZN4HPHP25f_image_type_to_extensionEib");

TypedValue* fh_getimagesize(TypedValue* _rv, Value* filename, TypedValue* imageinfo) asm("_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE");

Value* fh_gd_info(Value* _rv) asm("_ZN4HPHP9f_gd_infoEv");

TypedValue* fh_imageloadfont(TypedValue* _rv, Value* file) asm("_ZN4HPHP15f_imageloadfontERKNS_6StringE");

bool fh_imagesetstyle(Value* image, Value* style) asm("_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE");

TypedValue* fh_imagecreatetruecolor(TypedValue* _rv, int width, int height) asm("_ZN4HPHP22f_imagecreatetruecolorEii");

bool fh_imageistruecolor(Value* image) asm("_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE");

TypedValue* fh_imagetruecolortopalette(TypedValue* _rv, Value* image, bool dither, int ncolors) asm("_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi");

TypedValue* fh_imagecolormatch(TypedValue* _rv, Value* image1, Value* image2) asm("_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_");

bool fh_imagesetthickness(Value* image, int thickness) asm("_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi");

bool fh_imagefilledellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii");

bool fh_imagefilledarc(Value* image, int cx, int cy, int width, int height, int start, int end, int color, int style) asm("_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii");

bool fh_imagealphablending(Value* image, bool blendmode) asm("_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb");

bool fh_imagesavealpha(Value* image, bool saveflag) asm("_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb");

bool fh_imagelayereffect(Value* image, int effect) asm("_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi");

TypedValue* fh_imagecolorallocatealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii");

TypedValue* fh_imagecolorresolvealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii");

TypedValue* fh_imagecolorclosestalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii");

TypedValue* fh_imagecolorexactalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii");

bool fh_imagecopyresampled(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii");

TypedValue* fh_imagegrabwindow(TypedValue* _rv, int window, int client_area) asm("_ZN4HPHP17f_imagegrabwindowEii");

TypedValue* fh_imagegrabscreen(TypedValue* _rv) asm("_ZN4HPHP17f_imagegrabscreenEv");

TypedValue* fh_imagerotate(TypedValue* _rv, Value* source_image, double angle, int bgd_color, int ignore_transparent) asm("_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii");

bool fh_imagesettile(Value* image, Value* tile) asm("_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_");

bool fh_imagesetbrush(Value* image, Value* brush) asm("_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_");

TypedValue* fh_imagecreate(TypedValue* _rv, int width, int height) asm("_ZN4HPHP13f_imagecreateEii");

long fh_imagetypes() asm("_ZN4HPHP12f_imagetypesEv");

TypedValue* fh_imagecreatefromstring(TypedValue* _rv, Value* data) asm("_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE");

TypedValue* fh_imagecreatefromgif(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE");

TypedValue* fh_imagecreatefromjpeg(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE");

TypedValue* fh_imagecreatefrompng(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE");

TypedValue* fh_imagecreatefromxbm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE");

TypedValue* fh_imagecreatefromxpm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE");

TypedValue* fh_imagecreatefromwbmp(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE");

TypedValue* fh_imagecreatefromgd(TypedValue* _rv, Value* filename) asm("_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE");

TypedValue* fh_imagecreatefromgd2(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE");

TypedValue* fh_imagecreatefromgd2part(TypedValue* _rv, Value* filename, int srcx, int srcy, int width, int height) asm("_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii");

bool fh_imagexbm(Value* image, Value* filename, int foreground) asm("_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi");

bool fh_imagegif(Value* image, Value* filename) asm("_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE");

bool fh_imagepng(Value* image, Value* filename, int quality, int filters) asm("_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii");

bool fh_imagejpeg(Value* image, Value* filename, int quality) asm("_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi");

bool fh_imagewbmp(Value* image, Value* filename, int foreground) asm("_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi");

bool fh_imagegd(Value* image, Value* filename) asm("_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE");

bool fh_imagegd2(Value* image, Value* filename, int chunk_size, int type) asm("_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii");

bool fh_imagedestroy(Value* image) asm("_ZN4HPHP14f_imagedestroyERKNS_6ObjectE");

TypedValue* fh_imagecolorallocate(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii");

void fh_imagepalettecopy(Value* destination, Value* source) asm("_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_");

TypedValue* fh_imagecolorat(TypedValue* _rv, Value* image, int x, int y) asm("_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii");

TypedValue* fh_imagecolorclosest(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii");

TypedValue* fh_imagecolorclosesthwb(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii");

bool fh_imagecolordeallocate(Value* image, int color) asm("_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi");

TypedValue* fh_imagecolorresolve(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii");

TypedValue* fh_imagecolorexact(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii");

TypedValue* fh_imagecolorset(TypedValue* _rv, Value* image, int index, int red, int green, int blue) asm("_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii");

TypedValue* fh_imagecolorsforindex(TypedValue* _rv, Value* image, int index) asm("_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi");

bool fh_imagegammacorrect(Value* image, double inputgamma, double outputgamma) asm("_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd");

bool fh_imagesetpixel(Value* image, int x, int y, int color) asm("_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii");

bool fh_imageline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii");

bool fh_imagedashedline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii");

bool fh_imagerectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii");

bool fh_imagefilledrectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii");

bool fh_imagearc(Value* image, int cx, int cy, int width, int height, int start, int end, int color) asm("_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii");

bool fh_imageellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii");

bool fh_imagefilltoborder(Value* image, int x, int y, int border, int color) asm("_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii");

bool fh_imagefill(Value* image, int x, int y, int color) asm("_ZN4HPHP11f_imagefillERKNS_6ObjectEiii");

TypedValue* fh_imagecolorstotal(TypedValue* _rv, Value* image) asm("_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE");

TypedValue* fh_imagecolortransparent(TypedValue* _rv, Value* image, int color) asm("_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi");

TypedValue* fh_imageinterlace(TypedValue* _rv, Value* image, int interlace) asm("_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi");

bool fh_imagepolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii");

bool fh_imagefilledpolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii");

long fh_imagefontwidth(int font) asm("_ZN4HPHP16f_imagefontwidthEi");

long fh_imagefontheight(int font) asm("_ZN4HPHP17f_imagefontheightEi");

bool fh_imagechar(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi");

bool fh_imagecharup(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi");

bool fh_imagestring(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi");

bool fh_imagestringup(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi");

bool fh_imagecopy(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h) asm("_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii");

bool fh_imagecopymerge(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii");

bool fh_imagecopymergegray(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii");

bool fh_imagecopyresized(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii");

TypedValue* fh_imagesx(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesxERKNS_6ObjectE");

TypedValue* fh_imagesy(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesyERKNS_6ObjectE");

TypedValue* fh_imageftbbox(TypedValue* _rv, double size, double angle, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE");

TypedValue* fh_imagefttext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int col, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE");

TypedValue* fh_imagettfbbox(TypedValue* _rv, double size, double angle, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_");

TypedValue* fh_imagettftext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int color, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_");

Value* fh_imagepsloadfont(Value* _rv, Value* filename) asm("_ZN4HPHP17f_imagepsloadfontERKNS_6StringE");

bool fh_imagepsfreefont(Value* fontindex) asm("_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE");

bool fh_imagepsencodefont(Value* font_index, Value* encodingfile) asm("_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE");

bool fh_imagepsextendfont(int font_index, double extend) asm("_ZN4HPHP19f_imagepsextendfontEid");

bool fh_imagepsslantfont(Value* font_index, double slant) asm("_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd");

Value* fh_imagepstext(Value* _rv, Value* image, Value* text, Value* font, int size, int foreground, int background, int x, int y, int space, int tightness, double angle, int antialias_steps) asm("_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi");

Value* fh_imagepsbbox(Value* _rv, Value* text, int font, int size, int space, int tightness, double angle) asm("_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid");

bool fh_image2wbmp(Value* image, Value* filename, int threshold) asm("_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi");

bool fh_jpeg2wbmp(Value* jpegname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii");

bool fh_png2wbmp(Value* pngname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii");

bool fh_imagefilter(Value* image, int filtertype, int arg1, int arg2, int arg3, int arg4) asm("_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii");

bool fh_imageconvolution(Value* image, Value* matrix, double div, double offset) asm("_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd");

bool fh_imageantialias(Value* image, bool on) asm("_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb");

TypedValue* fh_iptcembed(TypedValue* _rv, Value* iptcdata, Value* jpeg_file_name, int spool) asm("_ZN4HPHP11f_iptcembedERKNS_6StringES2_i");

TypedValue* fh_iptcparse(TypedValue* _rv, Value* iptcblock) asm("_ZN4HPHP11f_iptcparseERKNS_6StringE");

TypedValue* fh_exif_tagname(TypedValue* _rv, int index) asm("_ZN4HPHP14f_exif_tagnameEi");

TypedValue* fh_exif_read_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb");

TypedValue* fh_read_exif_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb");

TypedValue* fh_exif_thumbnail(TypedValue* _rv, Value* filename, TypedValue* width, TypedValue* height, TypedValue* imagetype) asm("_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_");

TypedValue* fh_exif_imagetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_exif_imagetypeERKNS_6StringE");

} // namespace HPHP
