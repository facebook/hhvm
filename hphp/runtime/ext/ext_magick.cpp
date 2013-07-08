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

#include "hphp/runtime/ext/ext_magick.h"

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

int64_t f_magickgetversionnumber() {
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

Resource f_newdrawingwand() {
  throw NotImplementedException(__func__);
}

Resource f_newmagickwand() {
  throw NotImplementedException(__func__);
}

Resource f_newpixeliterator(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_newpixelregioniterator(CResRef mgck_wnd, int x, int y, int columns,
                                  int rows) {
  throw NotImplementedException(__func__);
}

Resource f_newpixelwand(CStrRef imagemagick_col_str /* = null_string */) {
  throw NotImplementedException(__func__);
}

Array f_newpixelwandarray(int num_pxl_wnds) {
  throw NotImplementedException(__func__);
}

Array f_newpixelwands(int num_pxl_wnds) {
  throw NotImplementedException(__func__);
}

void f_destroydrawingwand(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_destroymagickwand(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_destroypixeliterator(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_destroypixelwand(CResRef pxl_wnd) {
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

void f_cleardrawingwand(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_clearmagickwand(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_clearpixeliterator(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_clearpixelwand(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_clonedrawingwand(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_clonemagickwand(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_wandgetexception(CResRef wnd) {
  throw NotImplementedException(__func__);
}

String f_wandgetexceptionstring(CResRef wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_wandgetexceptiontype(CResRef wnd) {
  throw NotImplementedException(__func__);
}

bool f_wandhasexception(CResRef wnd) {
  throw NotImplementedException(__func__);
}

void f_drawaffine(CResRef drw_wnd, double sx, double sy, double rx, double ry,
                  double tx, double ty) {
  throw NotImplementedException(__func__);
}

void f_drawannotation(CResRef drw_wnd, double x, double y, CStrRef text) {
  throw NotImplementedException(__func__);
}

void f_drawarc(CResRef drw_wnd, double sx, double sy, double ex, double ey,
               double sd, double ed) {
  throw NotImplementedException(__func__);
}

void f_drawbezier(CResRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawcircle(CResRef drw_wnd, double ox, double oy, double px, double py) {
  throw NotImplementedException(__func__);
}

void f_drawcolor(CResRef drw_wnd, double x, double y, int paint_method) {
  throw NotImplementedException(__func__);
}

void f_drawcomment(CResRef drw_wnd, CStrRef comment) {
  throw NotImplementedException(__func__);
}

bool f_drawcomposite(CResRef drw_wnd, int composite_operator, double x,
                     double y, double width, double height, CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawellipse(CResRef drw_wnd, double ox, double oy, double rx, double ry,
                   double start, double end) {
  throw NotImplementedException(__func__);
}

String f_drawgetclippath(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetcliprule(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetclipunits(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Array f_drawgetexception(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetexceptionstring(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetexceptiontype(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfillalpha(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_drawgetfillcolor(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfillopacity(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetfillrule(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetfont(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetfontfamily(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfontsize(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetfontstretch(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetfontstyle(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetfontweight(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetgravity(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokealpha(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawgetstrokeantialias(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_drawgetstrokecolor(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Array f_drawgetstrokedasharray(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokedashoffset(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetstrokelinecap(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgetstrokelinejoin(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokemiterlimit(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokeopacity(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

double f_drawgetstrokewidth(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgettextalignment(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawgettextantialias(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_drawgettextdecoration(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgettextencoding(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_drawgettextundercolor(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

String f_drawgetvectorgraphics(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawline(CResRef drw_wnd, double sx, double sy, double ex, double ey) {
  throw NotImplementedException(__func__);
}

void f_drawmatte(CResRef drw_wnd, double x, double y, int paint_method) {
  throw NotImplementedException(__func__);
}

void f_drawpathclose(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoabsolute(CResRef drw_wnd, double x1, double y1, double x2,
                               double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbezierabsolute(CResRef drw_wnd,
                                              double x1,
                                              double y1,
                                              double x,
                                              double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbezierrelative(CResRef drw_wnd,
                                              double x1,
                                              double y1,
                                              double x,
                                              double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbeziersmoothabsolute(CResRef drw_wnd,
                                                    double x,
                                                    double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetoquadraticbeziersmoothrelative(CResRef drw_wnd,
                                                    double x,
                                                    double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetorelative(CResRef drw_wnd, double x1, double y1, double x2,
                               double y2, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetosmoothabsolute(CResRef drw_wnd, double x2, double y2,
                                     double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathcurvetosmoothrelative(CResRef drw_wnd, double x2, double y2,
                                     double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathellipticarcabsolute(CResRef drw_wnd, double rx, double ry,
                                   double x_axis_rotation, bool large_arc_flag,
                                   bool sweep_flag, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathellipticarcrelative(CResRef drw_wnd, double rx, double ry,
                                   double x_axis_rotation, bool large_arc_flag,
                                   bool sweep_flag, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathfinish(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoabsolute(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetohorizontalabsolute(CResRef drw_wnd, double x) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetohorizontalrelative(CResRef drw_wnd, double x) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetorelative(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoverticalabsolute(CResRef drw_wnd, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathlinetoverticalrelative(CResRef drw_wnd, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathmovetoabsolute(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathmovetorelative(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpathstart(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpoint(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_drawpolygon(CResRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawpolyline(CResRef drw_wnd, CArrRef x_y_points_array) {
  throw NotImplementedException(__func__);
}

void f_drawrectangle(CResRef drw_wnd, double x1, double y1, double x2,
                     double y2) {
  throw NotImplementedException(__func__);
}

bool f_drawrender(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawrotate(CResRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawroundrectangle(CResRef drw_wnd, double x1, double y1, double x2,
                          double y2, double rx, double ry) {
  throw NotImplementedException(__func__);
}

void f_drawscale(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_drawsetclippath(CResRef drw_wnd, CStrRef clip_path) {
  throw NotImplementedException(__func__);
}

void f_drawsetcliprule(CResRef drw_wnd, int fill_rule) {
  throw NotImplementedException(__func__);
}

void f_drawsetclipunits(CResRef drw_wnd, int clip_path_units) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillalpha(CResRef drw_wnd, double fill_opacity) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillcolor(CResRef drw_wnd, CResRef fill_pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillopacity(CResRef drw_wnd, double fill_opacity) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfillpatternurl(CResRef drw_wnd, CStrRef fill_url) {
  throw NotImplementedException(__func__);
}

void f_drawsetfillrule(CResRef drw_wnd, int fill_rule) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfont(CResRef drw_wnd, CStrRef font_file) {
  throw NotImplementedException(__func__);
}

bool f_drawsetfontfamily(CResRef drw_wnd, CStrRef font_family) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontsize(CResRef drw_wnd, double pointsize) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontstretch(CResRef drw_wnd, int stretch_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontstyle(CResRef drw_wnd, int style_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetfontweight(CResRef drw_wnd, double font_weight) {
  throw NotImplementedException(__func__);
}

void f_drawsetgravity(CResRef drw_wnd, int gravity_type) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokealpha(CResRef drw_wnd, double stroke_opacity) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokeantialias(CResRef drw_wnd,
                              bool stroke_antialias /* = true */) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokecolor(CResRef drw_wnd, CResRef strokecolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokedasharray(CResRef drw_wnd,
                              CArrRef dash_array /* = null_array */) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokedashoffset(CResRef drw_wnd, double dash_offset) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokelinecap(CResRef drw_wnd, int line_cap) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokelinejoin(CResRef drw_wnd, int line_join) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokemiterlimit(CResRef drw_wnd, double miterlimit) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokeopacity(CResRef drw_wnd, double stroke_opacity) {
  throw NotImplementedException(__func__);
}

bool f_drawsetstrokepatternurl(CResRef drw_wnd, CStrRef stroke_url) {
  throw NotImplementedException(__func__);
}

void f_drawsetstrokewidth(CResRef drw_wnd, double stroke_width) {
  throw NotImplementedException(__func__);
}

void f_drawsettextalignment(CResRef drw_wnd, int align_type) {
  throw NotImplementedException(__func__);
}

void f_drawsettextantialias(CResRef drw_wnd, bool text_antialias /* = true */) {
  throw NotImplementedException(__func__);
}

void f_drawsettextdecoration(CResRef drw_wnd, int decoration_type) {
  throw NotImplementedException(__func__);
}

void f_drawsettextencoding(CResRef drw_wnd, CStrRef encoding) {
  throw NotImplementedException(__func__);
}

void f_drawsettextundercolor(CResRef drw_wnd, CResRef undercolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_drawsetvectorgraphics(CResRef drw_wnd, CStrRef vector_graphics) {
  throw NotImplementedException(__func__);
}

void f_drawsetviewbox(CResRef drw_wnd, double x1, double y1, double x2,
                      double y2) {
  throw NotImplementedException(__func__);
}

void f_drawskewx(CResRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawskewy(CResRef drw_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

void f_drawtranslate(CResRef drw_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

void f_pushdrawingwand(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpushclippath(CResRef drw_wnd, CStrRef clip_path_id) {
  throw NotImplementedException(__func__);
}

void f_drawpushdefs(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpushpattern(CResRef drw_wnd, CStrRef pattern_id, double x, double y,
                       double width, double height) {
  throw NotImplementedException(__func__);
}

void f_popdrawingwand(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpopclippath(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpopdefs(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

void f_drawpoppattern(CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickadaptivethresholdimage(CResRef mgck_wnd, double width,
                                    double height, double offset) {
  throw NotImplementedException(__func__);
}

bool f_magickaddimage(CResRef mgck_wnd, CResRef add_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickaddnoiseimage(CResRef mgck_wnd, int noise_type) {
  throw NotImplementedException(__func__);
}

bool f_magickaffinetransformimage(CResRef mgck_wnd, CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickannotateimage(CResRef mgck_wnd, CResRef drw_wnd, double x,
                           double y, double angle, CStrRef text) {
  throw NotImplementedException(__func__);
}

Resource f_magickappendimages(CResRef mgck_wnd,
                              bool stack_vertical /* = false */) {
  throw NotImplementedException(__func__);
}

Resource f_magickaverageimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickblackthresholdimage(CResRef mgck_wnd, CResRef threshold_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickblurimage(CResRef mgck_wnd, double radius, double sigma,
                       int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickborderimage(CResRef mgck_wnd, CResRef bordercolor, double width,
                         double height) {
  throw NotImplementedException(__func__);
}

bool f_magickcharcoalimage(CResRef mgck_wnd, double radius, double sigma) {
  throw NotImplementedException(__func__);
}

bool f_magickchopimage(CResRef mgck_wnd, double width, double height, int x,
                       int y) {
  throw NotImplementedException(__func__);
}

bool f_magickclipimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickclippathimage(CResRef mgck_wnd, CStrRef pathname, bool inside) {
  throw NotImplementedException(__func__);
}

Resource f_magickcoalesceimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickcolorfloodfillimage(CResRef mgck_wnd, CResRef fillcolor_pxl_wnd,
                                 double fuzz, CResRef bordercolor_pxl_wnd,
                                 int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickcolorizeimage(CResRef mgck_wnd, CResRef colorize,
                           CResRef opacity_pxl_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magickcombineimages(CResRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

bool f_magickcommentimage(CResRef mgck_wnd, CStrRef comment) {
  throw NotImplementedException(__func__);
}

Array f_magickcompareimages(CResRef mgck_wnd, CResRef reference_wnd,
                            int metric_type, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickcompositeimage(CResRef mgck_wnd, CResRef composite_wnd,
                            int composite_operator, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickconstituteimage(CResRef mgck_wnd, double columns, double rows,
                             CStrRef smap, int storage_type,
                             CArrRef pixel_array) {
  throw NotImplementedException(__func__);
}

bool f_magickcontrastimage(CResRef mgck_wnd, bool sharpen) {
  throw NotImplementedException(__func__);
}

bool f_magickconvolveimage(CResRef mgck_wnd, CArrRef kernel_array,
                           int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickcropimage(CResRef mgck_wnd, double width, double height, int x,
                       int y) {
  throw NotImplementedException(__func__);
}

bool f_magickcyclecolormapimage(CResRef mgck_wnd, int num_positions) {
  throw NotImplementedException(__func__);
}

Resource f_magickdeconstructimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickdescribeimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickdespeckleimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickdrawimage(CResRef mgck_wnd, CResRef drw_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickechoimageblob(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickechoimagesblob(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickedgeimage(CResRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickembossimage(CResRef mgck_wnd, double radius, double sigma) {
  throw NotImplementedException(__func__);
}

bool f_magickenhanceimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickequalizeimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickevaluateimage(CResRef mgck_wnd, int evaluate_op, double constant,
                           int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

Resource f_magickflattenimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickflipimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickflopimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickframeimage(CResRef mgck_wnd, CResRef matte_color, double width,
                        double height, int inner_bevel, int outer_bevel) {
  throw NotImplementedException(__func__);
}

Resource f_magickfximage(CResRef mgck_wnd, CStrRef expression,
                         int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickgammaimage(CResRef mgck_wnd, double gamma,
                        int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickgaussianblurimage(CResRef mgck_wnd, double radius, double sigma,
                               int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

double f_magickgetcharheight(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                             bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgetcharwidth(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                            bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

Array f_magickgetexception(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetexceptionstring(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetexceptiontype(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetfilename(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetformat(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magickgetimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magickgetimagebackgroundcolor(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageblob(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageblueprimary(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magickgetimagebordercolor(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagechannelmean(CResRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

Resource f_magickgetimagecolormapcolor(CResRef mgck_wnd, double index) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagecolors(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagecolorspace(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagecompose(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagecompression(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagecompressionquality(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagedelay(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagedepth(CResRef mgck_wnd, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagedispose(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageextrema(CResRef mgck_wnd, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagefilename(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageformat(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagegamma(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagegreenprimary(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimageheight(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagehistogram(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimageindex(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimageinterlacescheme(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimageiterations(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magickgetimagemattecolor(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagemimetype(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagepixels(CResRef mgck_wnd, int x_offset, int y_offset,
                             double columns, double rows, CStrRef smap,
                             int storage_type) {
  throw NotImplementedException(__func__);
}

String f_magickgetimageprofile(CResRef mgck_wnd, CStrRef name) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageredprimary(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagerenderingintent(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimageresolution(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagescene(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagesignature(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagesize(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagetype(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimageunits(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetimagevirtualpixelmethod(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetimagewhitepoint(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetimagewidth(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickgetimagesblob(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_magickgetinterlacescheme(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetmaxtextadvance(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                                 bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

String f_magickgetmimetype(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetnumberimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetsamplingfactors(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

Array f_magickgetsize(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

double f_magickgetstringheight(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                               bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgetstringwidth(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                              bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgettextascent(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                             bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

double f_magickgettextdescent(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt,
                              bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

Array f_magickgetwandsize(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickhasnextimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickhaspreviousimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickimplodeimage(CResRef mgck_wnd, double amount) {
  throw NotImplementedException(__func__);
}

bool f_magicklabelimage(CResRef mgck_wnd, CStrRef label) {
  throw NotImplementedException(__func__);
}

bool f_magicklevelimage(CResRef mgck_wnd, double black_point, double gamma,
                        double white_point, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickmagnifyimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmapimage(CResRef mgck_wnd, CResRef map_wand, bool dither) {
  throw NotImplementedException(__func__);
}

bool f_magickmattefloodfillimage(CResRef mgck_wnd, double opacity, double fuzz,
                                 CResRef bordercolor_pxl_wnd, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickmedianfilterimage(CResRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickminifyimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmodulateimage(CResRef mgck_wnd, double brightness,
                           double saturation, double hue) {
  throw NotImplementedException(__func__);
}

Resource f_magickmontageimage(CResRef mgck_wnd,
                              CResRef drw_wnd,
                              CStrRef tile_geometry,
                              CStrRef thumbnail_geometry,
                              int montage_mode,
                              CStrRef frame) {
  throw NotImplementedException(__func__);
}

Resource f_magickmorphimages(CResRef mgck_wnd, double number_frames) {
  throw NotImplementedException(__func__);
}

Resource f_magickmosaicimages(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickmotionblurimage(CResRef mgck_wnd, double radius, double sigma,
                             double angle) {
  throw NotImplementedException(__func__);
}

bool f_magicknegateimage(CResRef mgck_wnd, bool only_the_gray /* = false */,
                         int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicknewimage(CResRef mgck_wnd, double width, double height,
                      CStrRef imagemagick_col_str /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magicknextimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicknormalizeimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickoilpaintimage(CResRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickpaintopaqueimage(CResRef mgck_wnd, CResRef target_pxl_wnd,
                              CResRef fill_pxl_wnd, double fuzz /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickpainttransparentimage(CResRef mgck_wnd,
                              CResRef target,
                              double opacity /* = k_MW_TransparentOpacity */,
                              double fuzz /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickpingimage(CResRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickposterizeimage(CResRef mgck_wnd, double levels, bool dither) {
  throw NotImplementedException(__func__);
}

Resource f_magickpreviewimages(CResRef mgck_wnd, int preview) {
  throw NotImplementedException(__func__);
}

bool f_magickpreviousimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickprofileimage(CResRef mgck_wnd, CStrRef name, CStrRef profile /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magickquantizeimage(CResRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  throw NotImplementedException(__func__);
}

bool f_magickquantizeimages(CResRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  throw NotImplementedException(__func__);
}

Array f_magickqueryfontmetrics(CResRef mgck_wnd, CResRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  throw NotImplementedException(__func__);
}

bool f_magickradialblurimage(CResRef mgck_wnd, double angle) {
  throw NotImplementedException(__func__);
}

bool f_magickraiseimage(CResRef mgck_wnd, double width, double height, int x, int y, bool raise) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimage(CResRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimageblob(CResRef mgck_wnd, CStrRef blob) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimagefile(CResRef mgck_wnd, CResRef handle) {
  throw NotImplementedException(__func__);
}

bool f_magickreadimages(CResRef mgck_wnd, CArrRef img_filenames_array) {
  throw NotImplementedException(__func__);
}

bool f_magickreducenoiseimage(CResRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

bool f_magickremoveimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

String f_magickremoveimageprofile(CResRef mgck_wnd, CStrRef name) {
  throw NotImplementedException(__func__);
}

bool f_magickremoveimageprofiles(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickresampleimage(CResRef mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) {
  throw NotImplementedException(__func__);
}

void f_magickresetiterator(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickresizeimage(CResRef mgck_wnd, double columns, double rows, int filter_type, double blur) {
  throw NotImplementedException(__func__);
}

bool f_magickrollimage(CResRef mgck_wnd, int x_offset, int y_offset) {
  throw NotImplementedException(__func__);
}

bool f_magickrotateimage(CResRef mgck_wnd, CResRef background, double degrees) {
  throw NotImplementedException(__func__);
}

bool f_magicksampleimage(CResRef mgck_wnd, double columns, double rows) {
  throw NotImplementedException(__func__);
}

bool f_magickscaleimage(CResRef mgck_wnd, double columns, double rows) {
  throw NotImplementedException(__func__);
}

bool f_magickseparateimagechannel(CResRef mgck_wnd, int channel_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetcompressionquality(CResRef mgck_wnd, double quality) {
  throw NotImplementedException(__func__);
}

bool f_magicksetfilename(CResRef mgck_wnd, CStrRef filename /* = null_string */) {
  throw NotImplementedException(__func__);
}

void f_magicksetfirstiterator(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetformat(CResRef mgck_wnd, CStrRef format) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimage(CResRef mgck_wnd, CResRef replace_wand) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebackgroundcolor(CResRef mgck_wnd, CResRef background_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebias(CResRef mgck_wnd, double bias) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageblueprimary(CResRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagebordercolor(CResRef mgck_wnd, CResRef border_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecolormapcolor(CResRef mgck_wnd, double index, CResRef mapcolor_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecolorspace(CResRef mgck_wnd, int colorspace_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompose(CResRef mgck_wnd, int composite_operator) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompression(CResRef mgck_wnd, int compression_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagecompressionquality(CResRef mgck_wnd, double quality) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedelay(CResRef mgck_wnd, double delay) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedepth(CResRef mgck_wnd, int depth, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagedispose(CResRef mgck_wnd, int dispose_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagefilename(CResRef mgck_wnd, CStrRef filename /* = null_string */) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageformat(CResRef mgck_wnd, CStrRef format) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagegamma(CResRef mgck_wnd, double gamma) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagegreenprimary(CResRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageindex(CResRef mgck_wnd, int index) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageinterlacescheme(CResRef mgck_wnd, int interlace_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageiterations(CResRef mgck_wnd, double iterations) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagemattecolor(CResRef mgck_wnd, CResRef matte_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageoption(CResRef mgck_wnd, CStrRef format, CStrRef key, CStrRef value) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagepixels(CResRef mgck_wnd, int x_offset, int y_offset, double columns, double rows, CStrRef smap, int storage_type, CArrRef pixel_array) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageprofile(CResRef mgck_wnd, CStrRef name, CStrRef profile) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageredprimary(CResRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagerenderingintent(CResRef mgck_wnd, int rendering_intent) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageresolution(CResRef mgck_wnd, double x_resolution, double y_resolution) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagescene(CResRef mgck_wnd, double scene) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagetype(CResRef mgck_wnd, int image_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimageunits(CResRef mgck_wnd, int resolution_type) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagevirtualpixelmethod(CResRef mgck_wnd, int virtual_pixel_method) {
  throw NotImplementedException(__func__);
}

bool f_magicksetimagewhitepoint(CResRef mgck_wnd, double x, double y) {
  throw NotImplementedException(__func__);
}

bool f_magicksetinterlacescheme(CResRef mgck_wnd, int interlace_type) {
  throw NotImplementedException(__func__);
}

void f_magicksetlastiterator(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magicksetpassphrase(CResRef mgck_wnd, CStrRef passphrase) {
  throw NotImplementedException(__func__);
}

bool f_magicksetresolution(CResRef mgck_wnd, double x_resolution, double y_resolution) {
  throw NotImplementedException(__func__);
}

bool f_magicksetsamplingfactors(CResRef mgck_wnd, double number_factors, CArrRef sampling_factors) {
  throw NotImplementedException(__func__);
}

bool f_magicksetsize(CResRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magicksetwandsize(CResRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magicksharpenimage(CResRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickshaveimage(CResRef mgck_wnd, int columns, int rows) {
  throw NotImplementedException(__func__);
}

bool f_magickshearimage(CResRef mgck_wnd, CResRef background, double x_shear, double y_shear) {
  throw NotImplementedException(__func__);
}

bool f_magicksolarizeimage(CResRef mgck_wnd, double threshold) {
  throw NotImplementedException(__func__);
}

bool f_magickspliceimage(CResRef mgck_wnd, double width, double height, int x, int y) {
  throw NotImplementedException(__func__);
}

bool f_magickspreadimage(CResRef mgck_wnd, double radius) {
  throw NotImplementedException(__func__);
}

Resource f_magicksteganoimage(CResRef mgck_wnd, CResRef watermark_wand, int offset) {
  throw NotImplementedException(__func__);
}

bool f_magickstereoimage(CResRef mgck_wnd, CResRef offset_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickstripimage(CResRef mgck_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickswirlimage(CResRef mgck_wnd, double degrees) {
  throw NotImplementedException(__func__);
}

Resource f_magicktextureimage(CResRef mgck_wnd, CResRef texture_wand) {
  throw NotImplementedException(__func__);
}

bool f_magickthresholdimage(CResRef mgck_wnd, double threshold, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magicktintimage(CResRef mgck_wnd, int tint_pxl_wnd, CResRef opacity_pxl_wnd) {
  throw NotImplementedException(__func__);
}

Resource f_magicktransformimage(CResRef mgck_wnd, CStrRef crop, CStrRef geometry) {
  throw NotImplementedException(__func__);
}

bool f_magicktrimimage(CResRef mgck_wnd, double fuzz) {
  throw NotImplementedException(__func__);
}

bool f_magickunsharpmaskimage(CResRef mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type /* = 0 */) {
  throw NotImplementedException(__func__);
}

bool f_magickwaveimage(CResRef mgck_wnd, double amplitude, double wave_length) {
  throw NotImplementedException(__func__);
}

bool f_magickwhitethresholdimage(CResRef mgck_wnd, CResRef threshold_pxl_wnd) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimage(CResRef mgck_wnd, CStrRef filename) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimagefile(CResRef mgck_wnd, CResRef handle) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimages(CResRef mgck_wnd, CStrRef filename /* = "" */, bool join_images /* = false */) {
  throw NotImplementedException(__func__);
}

bool f_magickwriteimagesfile(CResRef mgck_wnd, CResRef handle) {
  throw NotImplementedException(__func__);
}

double f_pixelgetalpha(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetalphaquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblack(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblackquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetblue(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetbluequantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

String f_pixelgetcolorasstring(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcolorcount(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcyan(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetcyanquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetexception(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

String f_pixelgetexceptionstring(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

int64_t f_pixelgetexceptiontype(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetgreen(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetgreenquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetindex(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetmagenta(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetmagentaquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetopacity(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetopacityquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetquantumcolor(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetred(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetredquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetyellow(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

double f_pixelgetyellowquantum(CResRef pxl_wnd) {
  throw NotImplementedException(__func__);
}

void f_pixelsetalpha(CResRef pxl_wnd, double alpha) {
  throw NotImplementedException(__func__);
}

void f_pixelsetalphaquantum(CResRef pxl_wnd, double alpha) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblack(CResRef pxl_wnd, double black) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblackquantum(CResRef pxl_wnd, double black) {
  throw NotImplementedException(__func__);
}

void f_pixelsetblue(CResRef pxl_wnd, double blue) {
  throw NotImplementedException(__func__);
}

void f_pixelsetbluequantum(CResRef pxl_wnd, double blue) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcolor(CResRef pxl_wnd, CStrRef imagemagick_col_str) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcolorcount(CResRef pxl_wnd, int count) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcyan(CResRef pxl_wnd, double cyan) {
  throw NotImplementedException(__func__);
}

void f_pixelsetcyanquantum(CResRef pxl_wnd, double cyan) {
  throw NotImplementedException(__func__);
}

void f_pixelsetgreen(CResRef pxl_wnd, double green) {
  throw NotImplementedException(__func__);
}

void f_pixelsetgreenquantum(CResRef pxl_wnd, double green) {
  throw NotImplementedException(__func__);
}

void f_pixelsetindex(CResRef pxl_wnd, double index) {
  throw NotImplementedException(__func__);
}

void f_pixelsetmagenta(CResRef pxl_wnd, double magenta) {
  throw NotImplementedException(__func__);
}

void f_pixelsetmagentaquantum(CResRef pxl_wnd, double magenta) {
  throw NotImplementedException(__func__);
}

void f_pixelsetopacity(CResRef pxl_wnd, double opacity) {
  throw NotImplementedException(__func__);
}

void f_pixelsetopacityquantum(CResRef pxl_wnd, double opacity) {
  throw NotImplementedException(__func__);
}

void f_pixelsetquantumcolor(CResRef pxl_wnd, double red, double green, double blue, double opacity /* = 0.0 */) {
  throw NotImplementedException(__func__);
}

void f_pixelsetred(CResRef pxl_wnd, double red) {
  throw NotImplementedException(__func__);
}

void f_pixelsetredquantum(CResRef pxl_wnd, double red) {
  throw NotImplementedException(__func__);
}

void f_pixelsetyellow(CResRef pxl_wnd, double yellow) {
  throw NotImplementedException(__func__);
}

void f_pixelsetyellowquantum(CResRef pxl_wnd, double yellow) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetiteratorexception(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

String f_pixelgetiteratorexceptionstring(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

int64_t f_pixelgetiteratorexceptiontype(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

Array f_pixelgetnextiteratorrow(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

void f_pixelresetiterator(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}

bool f_pixelsetiteratorrow(CResRef pxl_iter, int row) {
  throw NotImplementedException(__func__);
}

bool f_pixelsynciterator(CResRef pxl_iter) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
