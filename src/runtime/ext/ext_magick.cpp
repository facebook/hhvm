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

#include <runtime/ext/ext_magick.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String f_magickgetcopyright() {
  throw NotImplementedException(__func__);
}

String f_magickgethomeurl() {
  throw NotImplementedException(__func__);
}

String f_magickgetpackagename() {
  throw NotImplementedException(__func__);
}

double f_magickgetquantumdepth() {
  throw NotImplementedException(__func__);
}

String f_magickgetreleasedate() {
  throw NotImplementedException(__func__);
}

double f_magickgetresourcelimit(int resource_type) {
  throw NotImplementedException(__func__);
}

Array f_magickgetversion() {
  throw NotImplementedException(__func__);
}

int f_magickgetversionnumber() {
  throw NotImplementedException(__func__);
}

String f_magickgetversionstring() {
  throw NotImplementedException(__func__);
}

String f_magickqueryconfigureoption(CStrRef option) {
  throw NotImplementedException(__func__);
}

Array f_magickqueryconfigureoptions(CStrRef pattern) {
  throw NotImplementedException(__func__);
}

Array f_magickqueryfonts(CStrRef pattern) {
  throw NotImplementedException(__func__);
}

Array f_magickqueryformats(CStrRef pattern) {
  throw NotImplementedException(__func__);
}

bool f_magicksetresourcelimit(int resource_type, double limit) {
  throw NotImplementedException(__func__);
}

Object f_newdrawingwand() {
  throw NotImplementedException(__func__);
}

Object f_newmagickwand() {
  throw NotImplementedException(__func__);
}

Object f_newpixeliterator(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Object f_newpixelregioniterator(CObjRef mgck_wnd, int x, int y, int columns, int rows) {
  throw NotImplementedException(__func__);
}

Object f_newpixelwand(CStrRef imagemagick_col_str /* = null_string */) {
  throw NotImplementedException(__func__);
}

Array f_newpixelwandarray(int num_pxl_wnds) {
  throw NotImplementedException(__func__);
}

Array f_newpixelwands(int num_pxl_wnds) {
  throw NotImplementedException(__func__);
}

void f_destroydrawingwand(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_destroymagickwand(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_destroypixeliterator(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_destroypixelwand(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_destroypixelwandarray(CArrRef pxl_wnd_array) {
  throw NotImplementedException(__func__);
}

void f_destroypixelwands(CArrRef pxl_wnd_array) {
  throw NotImplementedException(__func__);
}

bool f_isdrawingwand(CVarRef var) {
  throw NotImplementedException(__func__);
}

bool f_ismagickwand(CVarRef var) {
  throw NotImplementedException(__func__);
}

bool f_ispixeliterator(CVarRef var) {
  throw NotImplementedException(__func__);
}

bool f_ispixelwand(CVarRef var) {
  throw NotImplementedException(__func__);
}

void f_cleardrawingwand(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_clearmagickwand(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_clearpixeliterator(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_clearpixelwand(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Object f_clonedrawingwand(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Object f_clonemagickwand(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_wandgetexception(CObjRef wnd) {
  throw NotImplementedException(__func__);
}

String f_wandgetexceptionstring(CObjRef wnd) {
  throw NotImplementedException(__func__);
}

int f_wandgetexceptiontype(CObjRef wnd) {
  throw NotImplementedException(__func__);
}

bool f_wandhasexception(CObjRef wnd) {
  throw NotImplementedException(__func__);
}

void f_drawaffine(CObjRef drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) {
  throw NotImplementedException(__func__);
}

void f_drawannotation(CObjRef drw_wnd, double x, double y, CStrRef text) {
  throw NotImplementedException(__func__);
}

void f_drawarc(CObjRef drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) {
  throw NotImplementedException(__func__);
}

void f_drawbezier(CObjRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawcircle(CObjRef drw_wnd, double ox, double oy, double px, double py) {
  throw NotImplementedException(__func__);
}

void f_drawcolor(CObjRef drw_wnd, double x, double y, int paint_method) {
  throw NotImplementedException(__func__);
}

void f_drawcomment(CObjRef drw_wnd, CStrRef comment) {
  throw NotImplementedException(__func__);
}

bool f_drawcomposite(CObjRef drw_wnd, int composite_operator, double x, double y, double width, double height, CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawellipse(CObjRef drw_wnd, double ox, double oy, double rx, double ry, double start, double end) {
  throw NotImplementedException(__func__);
}

String f_drawgetclippath(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetcliprule(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetclipunits(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Array f_drawgetexception(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetexceptionstring(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetexceptiontype(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfillalpha(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Object f_drawgetfillcolor(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfillopacity(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetfillrule(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetfont(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetfontfamily(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfontsize(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetfontstretch(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetfontstyle(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfontweight(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetgravity(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokealpha(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawgetstrokeantialias(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Object f_drawgetstrokecolor(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Array f_drawgetstrokedasharray(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokedashoffset(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetstrokelinecap(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgetstrokelinejoin(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokemiterlimit(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokeopacity(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokewidth(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgettextalignment(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawgettextantialias(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int f_drawgettextdecoration(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgettextencoding(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Object f_drawgettextundercolor(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetvectorgraphics(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawline(CObjRef drw_wnd, double sx, double sy, double ex, double ey) {
  throw NotImplementedException(__func__);
}

void f_drawmatte(CObjRef drw_wnd, double x, double y, int paint_method) {
  throw NotImplementedException(__func__);
}

void f_drawpathclose(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoabsolute(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbezierabsolute(CObjRef drw_wnd, double x1, double y1, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbezierrelative(CObjRef drw_wnd, double x1, double y1, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbeziersmoothabsolute(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbeziersmoothrelative(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetorelative(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetosmoothabsolute(CObjRef drw_wnd, double x2, double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetosmoothrelative(CObjRef drw_wnd, double x2, double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathellipticarcabsolute(CObjRef drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathellipticarcrelative(CObjRef drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathfinish(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoabsolute(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetohorizontalabsolute(CObjRef drw_wnd, double x) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetohorizontalrelative(CObjRef drw_wnd, double x) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetorelative(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoverticalabsolute(CObjRef drw_wnd, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoverticalrelative(CObjRef drw_wnd, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathmovetoabsolute(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathmovetorelative(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathstart(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpoint(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpolygon(CObjRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawpolyline(CObjRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawrectangle(CObjRef drw_wnd, double x1, double y1, double x2, double y2) {
  throw NotImplementedException(__func__);
}

bool f_drawrender(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawrotate(CObjRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawroundrectangle(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) {
  throw NotImplementedException(__func__);
}

void f_drawscale(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_drawsetclippath(CObjRef drw_wnd, CStrRef clip_path) {
  throw NotImplementedException(__func__);
}

void f_drawsetcliprule(CObjRef drw_wnd, int fill_rule) {
  throw NotImplementedException(__func__);
}

void f_drawsetclipunits(CObjRef drw_wnd, int clip_path_units) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillalpha(CObjRef drw_wnd, double fill_opacity) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillcolor(CObjRef drw_wnd, CObjRef fill_pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillopacity(CObjRef drw_wnd, double fill_opacity) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfillpatternurl(CObjRef drw_wnd, CStrRef fill_url) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillrule(CObjRef drw_wnd, int fill_rule) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfont(CObjRef drw_wnd, CStrRef font_file) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfontfamily(CObjRef drw_wnd, CStrRef font_family) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontsize(CObjRef drw_wnd, double pointsize) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontstretch(CObjRef drw_wnd, int stretch_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontstyle(CObjRef drw_wnd, int style_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontweight(CObjRef drw_wnd, double font_weight) {
  throw NotImplementedException(__func__);
}

void f_drawsetgravity(CObjRef drw_wnd, int gravity_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokealpha(CObjRef drw_wnd, double stroke_opacity) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokeantialias(CObjRef drw_wnd, bool stroke_antialias /* = true */) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokecolor(CObjRef drw_wnd, CObjRef strokecolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokedasharray(CObjRef drw_wnd, CArrRef dash_array /* = null_array */) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokedashoffset(CObjRef drw_wnd, double dash_offset) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokelinecap(CObjRef drw_wnd, int line_cap) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokelinejoin(CObjRef drw_wnd, int line_join) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokemiterlimit(CObjRef drw_wnd, double miterlimit) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokeopacity(CObjRef drw_wnd, double stroke_opacity) {
  throw NotImplementedException(__func__);
}

bool f_drawsetstrokepatternurl(CObjRef drw_wnd, CStrRef stroke_url) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokewidth(CObjRef drw_wnd, double stroke_width) {
  throw NotImplementedException(__func__);
}

void f_drawsettextalignment(CObjRef drw_wnd, int align_type) {
  throw NotImplementedException(__func__);
}

void f_drawsettextantialias(CObjRef drw_wnd, bool text_antialias /* = true */) {
  throw NotImplementedException(__func__);
}

void f_drawsettextdecoration(CObjRef drw_wnd, int decoration_type) {
  throw NotImplementedException(__func__);
}

void f_drawsettextencoding(CObjRef drw_wnd, CStrRef encoding) {
  throw NotImplementedException(__func__);
}

void f_drawsettextundercolor(CObjRef drw_wnd, CObjRef undercolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawsetvectorgraphics(CObjRef drw_wnd, CStrRef vector_graphics) {
  throw NotImplementedException(__func__);
}

void f_drawsetviewbox(CObjRef drw_wnd, double x1, double y1, double x2, double y2) {
  throw NotImplementedException(__func__);
}

void f_drawskewx(CObjRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawskewy(CObjRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawtranslate(CObjRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_pushdrawingwand(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpushclippath(CObjRef drw_wnd, CStrRef clip_path_id) {
  throw NotImplementedException(__func__);
}

void f_drawpushdefs(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpushpattern(CObjRef drw_wnd, CStrRef pattern_id, double x, double y, double width, double height) {
  throw NotImplementedException(__func__);
}

void f_popdrawingwand(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpopclippath(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpopdefs(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpoppattern(CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickadaptivethresholdimage(CObjRef mgck_wnd, double width, double height, double offset) {
  throw NotImplementedException(__func__);
}

bool f_magickaddimage(CObjRef mgck_wnd, CObjRef add_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickaddnoiseimage(CObjRef mgck_wnd, int noise_type) {
  throw NotImplementedException(__func__);
}

bool f_magickaffinetransformimage(CObjRef mgck_wnd, CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickannotateimage(CObjRef mgck_wnd, CObjRef drw_wnd, double x, double y, double angle, CStrRef text) {
  throw NotImplementedException(__func__);
}

Object f_magickappendimages(CObjRef mgck_wnd, bool stack_vertical /* = false */) {
  throw NotImplementedException(__func__);
}

Object f_magickaverageimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickblackthresholdimage(CObjRef mgck_wnd, CObjRef threshold_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickblurimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickborderimage(CObjRef mgck_wnd, CObjRef bordercolor, double width, double height) {
  throw NotImplementedException(__func__);
}

bool f_magickcharcoalimage(CObjRef mgck_wnd, double radius, double sigma) {
  throw NotImplementedException(__func__);
}

bool f_magickchopimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickclipimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickclippathimage(CObjRef mgck_wnd, CStrRef pathname, bool inside) {
  throw NotImplementedException(__func__);
}

Object f_magickcoalesceimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickcolorfloodfillimage(CObjRef mgck_wnd, CObjRef fillcolor_pxl_wnd, double fuzz, CObjRef bordercolor_pxl_wnd, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickcolorizeimage(CObjRef mgck_wnd, CObjRef colorize, CObjRef opacity_pxl_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magickcombineimages(CObjRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

bool f_magickcommentimage(CObjRef mgck_wnd, CStrRef comment) {
  throw NotImplementedException(__func__);
}

Array f_magickcompareimages(CObjRef mgck_wnd, CObjRef reference_wnd, int metric_type, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickcompositeimage(CObjRef mgck_wnd, CObjRef composite_wnd, int composite_operator, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickconstituteimage(CObjRef mgck_wnd, double columns, double rows, CStrRef smap, int storage_type, CArrRef pixel_array) {
  throw NotImplementedException(__func__);
}

bool f_magickcontrastimage(CObjRef mgck_wnd, bool sharpen) {
  throw NotImplementedException(__func__);
}

bool f_magickconvolveimage(CObjRef mgck_wnd, CArrRef kernel_array, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickcropimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickcyclecolormapimage(CObjRef mgck_wnd, int num_positions) {
  throw NotImplementedException(__func__);
}

Object f_magickdeconstructimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickdescribeimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickdespeckleimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickdrawimage(CObjRef mgck_wnd, CObjRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickechoimageblob(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickechoimagesblob(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickedgeimage(CObjRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickembossimage(CObjRef mgck_wnd, double radius, double sigma) {
  throw NotImplementedException(__func__);
}

bool f_magickenhanceimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickequalizeimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickevaluateimage(CObjRef mgck_wnd, int evaluate_op, double constant, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

Object f_magickflattenimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickflipimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickflopimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickframeimage(CObjRef mgck_wnd, CObjRef matte_color, double width, double height, int inner_bevel, int outer_bevel) {
  throw NotImplementedException(__func__);
}

Object f_magickfximage(CObjRef mgck_wnd, CStrRef expression, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickgammaimage(CObjRef mgck_wnd, double gamma, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickgaussianblurimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

double f_magickgetcharheight(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgetcharwidth(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

Array f_magickgetexception(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetexceptionstring(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetexceptiontype(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetfilename(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetformat(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magickgetimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magickgetimagebackgroundcolor(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageblob(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageblueprimary(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magickgetimagebordercolor(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagechannelmean(CObjRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

Object f_magickgetimagecolormapcolor(CObjRef mgck_wnd, double index) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagecolors(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagecolorspace(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagecompose(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagecompression(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagecompressionquality(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagedelay(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagedepth(CObjRef mgck_wnd, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagedispose(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageextrema(CObjRef mgck_wnd, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagefilename(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageformat(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagegamma(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagegreenprimary(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimageheight(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagehistogram(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimageindex(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimageinterlacescheme(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimageiterations(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magickgetimagemattecolor(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagemimetype(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagepixels(CObjRef mgck_wnd, int x_offset, int y_offset, double columns, double rows, CStrRef smap, int storage_type) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageprofile(CObjRef mgck_wnd, CStrRef name) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageredprimary(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagerenderingintent(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageresolution(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagescene(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagesignature(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagesize(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagetype(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimageunits(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetimagevirtualpixelmethod(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagewhitepoint(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagewidth(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagesblob(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int f_magickgetinterlacescheme(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetmaxtextadvance(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

String f_magickgetmimetype(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetnumberimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetsamplingfactors(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetsize(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetstringheight(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgetstringwidth(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgettextascent(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgettextdescent(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

Array f_magickgetwandsize(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickhasnextimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickhaspreviousimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickimplodeimage(CObjRef mgck_wnd, double amount) {
  throw NotImplementedException(__func__);
}

bool f_magicklabelimage(CObjRef mgck_wnd, CStrRef label) {
  throw NotImplementedException(__func__);
}

bool f_magicklevelimage(CObjRef mgck_wnd, double black_point, double gamma, double white_point, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickmagnifyimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmapimage(CObjRef mgck_wnd, CObjRef map_wand, bool dither) {
  throw NotImplementedException(__func__);
}

bool f_magickmattefloodfillimage(CObjRef mgck_wnd, double opacity, double fuzz, CObjRef bordercolor_pxl_wnd, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickmedianfilterimage(CObjRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickminifyimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmodulateimage(CObjRef mgck_wnd, double brightness, double saturation, double hue) {
  throw NotImplementedException(__func__);
}

Object f_magickmontageimage(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef tile_geometry, CStrRef thumbnail_geometry, int montage_mode, CStrRef frame) {
  throw NotImplementedException(__func__);
}

Object f_magickmorphimages(CObjRef mgck_wnd, double number_frames) {
  throw NotImplementedException(__func__);
}

Object f_magickmosaicimages(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmotionblurimage(CObjRef mgck_wnd, double radius, double sigma, double angle) {
  throw NotImplementedException(__func__);
}

bool f_magicknegateimage(CObjRef mgck_wnd, bool only_the_gray /* = false */, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicknewimage(CObjRef mgck_wnd, double width, double height, CStrRef imagemagick_col_str /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magicknextimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicknormalizeimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickoilpaintimage(CObjRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickpaintopaqueimage(CObjRef mgck_wnd, CObjRef target_pxl_wnd, CObjRef fill_pxl_wnd, double fuzz /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickpainttransparentimage(CObjRef mgck_wnd, CObjRef target, double opacity /* = k_MW_TransparentOpacity */, double fuzz /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickpingimage(CObjRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickposterizeimage(CObjRef mgck_wnd, double levels, bool dither) {
  throw NotImplementedException(__func__);
}

Object f_magickpreviewimages(CObjRef mgck_wnd, int preview) {
  throw NotImplementedException(__func__);
}

bool f_magickpreviousimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickprofileimage(CObjRef mgck_wnd, CStrRef name, CStrRef profile /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magickquantizeimage(CObjRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  throw NotImplementedException(__func__);
}

bool f_magickquantizeimages(CObjRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  throw NotImplementedException(__func__);
}

Array f_magickqueryfontmetrics(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

bool f_magickradialblurimage(CObjRef mgck_wnd, double angle) {
  throw NotImplementedException(__func__);
}

bool f_magickraiseimage(CObjRef mgck_wnd, double width, double height, int x, int y, bool raise) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimage(CObjRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimageblob(CObjRef mgck_wnd, CStrRef blob) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimagefile(CObjRef mgck_wnd, CObjRef handle) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimages(CObjRef mgck_wnd, CArrRef img_filenames_array) {
  throw NotImplementedException(__func__);
}

bool f_magickreducenoiseimage(CObjRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickremoveimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickremoveimageprofile(CObjRef mgck_wnd, CStrRef name) {
  throw NotImplementedException(__func__);
}

bool f_magickremoveimageprofiles(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickresampleimage(CObjRef mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) {
  throw NotImplementedException(__func__);
}

void f_magickresetiterator(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickresizeimage(CObjRef mgck_wnd, double columns, double rows, int filter_type, double blur) {
  throw NotImplementedException(__func__);
}

bool f_magickrollimage(CObjRef mgck_wnd, int x_offset, int y_offset) {
  throw NotImplementedException(__func__);
}

bool f_magickrotateimage(CObjRef mgck_wnd, CObjRef background, double degrees) {
  throw NotImplementedException(__func__);
}

bool f_magicksampleimage(CObjRef mgck_wnd, double columns, double rows) {
  throw NotImplementedException(__func__);
}

bool f_magickscaleimage(CObjRef mgck_wnd, double columns, double rows) {
  throw NotImplementedException(__func__);
}

bool f_magickseparateimagechannel(CObjRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetcompressionquality(CObjRef mgck_wnd, double quality) {
  throw NotImplementedException(__func__);
}

bool f_magicksetfilename(CObjRef mgck_wnd, CStrRef filename /* = null_string */) {
  throw NotImplementedException(__func__);
}

void f_magicksetfirstiterator(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetformat(CObjRef mgck_wnd, CStrRef format) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimage(CObjRef mgck_wnd, CObjRef replace_wand) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebackgroundcolor(CObjRef mgck_wnd, CObjRef background_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebias(CObjRef mgck_wnd, double bias) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageblueprimary(CObjRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebordercolor(CObjRef mgck_wnd, CObjRef border_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecolormapcolor(CObjRef mgck_wnd, double index, CObjRef mapcolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecolorspace(CObjRef mgck_wnd, int colorspace_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompose(CObjRef mgck_wnd, int composite_operator) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompression(CObjRef mgck_wnd, int compression_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompressionquality(CObjRef mgck_wnd, double quality) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedelay(CObjRef mgck_wnd, double delay) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedepth(CObjRef mgck_wnd, int depth, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedispose(CObjRef mgck_wnd, int dispose_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagefilename(CObjRef mgck_wnd, CStrRef filename /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageformat(CObjRef mgck_wnd, CStrRef format) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagegamma(CObjRef mgck_wnd, double gamma) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagegreenprimary(CObjRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageindex(CObjRef mgck_wnd, int index) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageinterlacescheme(CObjRef mgck_wnd, int interlace_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageiterations(CObjRef mgck_wnd, double iterations) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagemattecolor(CObjRef mgck_wnd, CObjRef matte_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageoption(CObjRef mgck_wnd, CStrRef format, CStrRef key, CStrRef value) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagepixels(CObjRef mgck_wnd, int x_offset, int y_offset, double columns, double rows, CStrRef smap, int storage_type, CArrRef pixel_array) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageprofile(CObjRef mgck_wnd, CStrRef name, CStrRef profile) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageredprimary(CObjRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagerenderingintent(CObjRef mgck_wnd, int rendering_intent) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageresolution(CObjRef mgck_wnd, double x_resolution, double y_resolution) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagescene(CObjRef mgck_wnd, double scene) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagetype(CObjRef mgck_wnd, int image_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageunits(CObjRef mgck_wnd, int resolution_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagevirtualpixelmethod(CObjRef mgck_wnd, int virtual_pixel_method) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagewhitepoint(CObjRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetinterlacescheme(CObjRef mgck_wnd, int interlace_type) {
  throw NotImplementedException(__func__);
}

void f_magicksetlastiterator(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetpassphrase(CObjRef mgck_wnd, CStrRef passphrase) {
  throw NotImplementedException(__func__);
}

bool f_magicksetresolution(CObjRef mgck_wnd, double x_resolution, double y_resolution) {
  throw NotImplementedException(__func__);
}

bool f_magicksetsamplingfactors(CObjRef mgck_wnd, double number_factors, CArrRef sampling_factors) {
  throw NotImplementedException(__func__);
}

bool f_magicksetsize(CObjRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magicksetwandsize(CObjRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magicksharpenimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickshaveimage(CObjRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magickshearimage(CObjRef mgck_wnd, CObjRef background, double x_shear, double y_shear) {
  throw NotImplementedException(__func__);
}

bool f_magicksolarizeimage(CObjRef mgck_wnd, double threshold) {
  throw NotImplementedException(__func__);
}

bool f_magickspliceimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickspreadimage(CObjRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

Object f_magicksteganoimage(CObjRef mgck_wnd, CObjRef watermark_wand, int offset) {
  throw NotImplementedException(__func__);
}

bool f_magickstereoimage(CObjRef mgck_wnd, CObjRef offset_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickstripimage(CObjRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickswirlimage(CObjRef mgck_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

Object f_magicktextureimage(CObjRef mgck_wnd, CObjRef texture_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickthresholdimage(CObjRef mgck_wnd, double threshold, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicktintimage(CObjRef mgck_wnd, int tint_pxl_wnd, CObjRef opacity_pxl_wnd) {
  throw NotImplementedException(__func__);
}

Object f_magicktransformimage(CObjRef mgck_wnd, CStrRef crop, CStrRef geometry) {
  throw NotImplementedException(__func__);
}

bool f_magicktrimimage(CObjRef mgck_wnd, double fuzz) {
  throw NotImplementedException(__func__);
}

bool f_magickunsharpmaskimage(CObjRef mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickwaveimage(CObjRef mgck_wnd, double amplitude, double wave_length) {
  throw NotImplementedException(__func__);
}

bool f_magickwhitethresholdimage(CObjRef mgck_wnd, CObjRef threshold_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimage(CObjRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimagefile(CObjRef mgck_wnd, CObjRef handle) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimages(CObjRef mgck_wnd, CStrRef filename /* = "" */, bool join_images /* = false */) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimagesfile(CObjRef mgck_wnd, CObjRef handle) {
  throw NotImplementedException(__func__);
}

double f_pixelgetalpha(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetalphaquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblack(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblackquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblue(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetbluequantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

String f_pixelgetcolorasstring(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcolorcount(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcyan(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcyanquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetexception(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

String f_pixelgetexceptionstring(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

int f_pixelgetexceptiontype(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetgreen(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetgreenquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetindex(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetmagenta(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetmagentaquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetopacity(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetopacityquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetquantumcolor(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetred(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetredquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetyellow(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetyellowquantum(CObjRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_pixelsetalpha(CObjRef pxl_wnd, double alpha) {
  throw NotImplementedException(__func__);
}

void f_pixelsetalphaquantum(CObjRef pxl_wnd, double alpha) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblack(CObjRef pxl_wnd, double black) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblackquantum(CObjRef pxl_wnd, double black) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblue(CObjRef pxl_wnd, double blue) {
  throw NotImplementedException(__func__);
}

void f_pixelsetbluequantum(CObjRef pxl_wnd, double blue) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcolor(CObjRef pxl_wnd, CStrRef imagemagick_col_str) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcolorcount(CObjRef pxl_wnd, int count) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcyan(CObjRef pxl_wnd, double cyan) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcyanquantum(CObjRef pxl_wnd, double cyan) {
  throw NotImplementedException(__func__);
}

void f_pixelsetgreen(CObjRef pxl_wnd, double green) {
  throw NotImplementedException(__func__);
}

void f_pixelsetgreenquantum(CObjRef pxl_wnd, double green) {
  throw NotImplementedException(__func__);
}

void f_pixelsetindex(CObjRef pxl_wnd, double index) {
  throw NotImplementedException(__func__);
}

void f_pixelsetmagenta(CObjRef pxl_wnd, double magenta) {
  throw NotImplementedException(__func__);
}

void f_pixelsetmagentaquantum(CObjRef pxl_wnd, double magenta) {
  throw NotImplementedException(__func__);
}

void f_pixelsetopacity(CObjRef pxl_wnd, double opacity) {
  throw NotImplementedException(__func__);
}

void f_pixelsetopacityquantum(CObjRef pxl_wnd, double opacity) {
  throw NotImplementedException(__func__);
}

void f_pixelsetquantumcolor(CObjRef pxl_wnd, double red, double green, double blue, double opacity /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

void f_pixelsetred(CObjRef pxl_wnd, double red) {
  throw NotImplementedException(__func__);
}

void f_pixelsetredquantum(CObjRef pxl_wnd, double red) {
  throw NotImplementedException(__func__);
}

void f_pixelsetyellow(CObjRef pxl_wnd, double yellow) {
  throw NotImplementedException(__func__);
}

void f_pixelsetyellowquantum(CObjRef pxl_wnd, double yellow) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetiteratorexception(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

String f_pixelgetiteratorexceptionstring(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

int f_pixelgetiteratorexceptiontype(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetnextiteratorrow(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_pixelresetiterator(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}

bool f_pixelsetiteratorrow(CObjRef pxl_iter, int row) {
  throw NotImplementedException(__func__);
}

bool f_pixelsynciterator(CObjRef pxl_iter) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
