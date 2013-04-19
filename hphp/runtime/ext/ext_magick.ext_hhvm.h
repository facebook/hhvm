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

Value* fh_magickgetcopyright(Value* _rv) asm("_ZN4HPHP20f_magickgetcopyrightEv");

Value* fh_magickgethomeurl(Value* _rv) asm("_ZN4HPHP18f_magickgethomeurlEv");

Value* fh_magickgetpackagename(Value* _rv) asm("_ZN4HPHP22f_magickgetpackagenameEv");

double fh_magickgetquantumdepth() asm("_ZN4HPHP23f_magickgetquantumdepthEv");

Value* fh_magickgetreleasedate(Value* _rv) asm("_ZN4HPHP22f_magickgetreleasedateEv");

double fh_magickgetresourcelimit(int resource_type) asm("_ZN4HPHP24f_magickgetresourcelimitEi");

Value* fh_magickgetversion(Value* _rv) asm("_ZN4HPHP18f_magickgetversionEv");

long fh_magickgetversionnumber() asm("_ZN4HPHP24f_magickgetversionnumberEv");

Value* fh_magickgetversionstring(Value* _rv) asm("_ZN4HPHP24f_magickgetversionstringEv");

Value* fh_magickqueryconfigureoption(Value* _rv, Value* option) asm("_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE");

Value* fh_magickqueryconfigureoptions(Value* _rv, Value* pattern) asm("_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE");

Value* fh_magickqueryfonts(Value* _rv, Value* pattern) asm("_ZN4HPHP18f_magickqueryfontsERKNS_6StringE");

Value* fh_magickqueryformats(Value* _rv, Value* pattern) asm("_ZN4HPHP20f_magickqueryformatsERKNS_6StringE");

bool fh_magicksetresourcelimit(int resource_type, double limit) asm("_ZN4HPHP24f_magicksetresourcelimitEid");

Value* fh_newdrawingwand(Value* _rv) asm("_ZN4HPHP16f_newdrawingwandEv");

Value* fh_newmagickwand(Value* _rv) asm("_ZN4HPHP15f_newmagickwandEv");

Value* fh_newpixeliterator(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE");

Value* fh_newpixelregioniterator(Value* _rv, Value* mgck_wnd, int x, int y, int columns, int rows) asm("_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii");

Value* fh_newpixelwand(Value* _rv, Value* imagemagick_col_str) asm("_ZN4HPHP14f_newpixelwandERKNS_6StringE");

Value* fh_newpixelwandarray(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP19f_newpixelwandarrayEi");

Value* fh_newpixelwands(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP15f_newpixelwandsEi");

void fh_destroydrawingwand(Value* drw_wnd) asm("_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE");

void fh_destroymagickwand(Value* mgck_wnd) asm("_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE");

void fh_destroypixeliterator(Value* pxl_iter) asm("_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE");

void fh_destroypixelwand(Value* pxl_wnd) asm("_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE");

void fh_destroypixelwandarray(Value* pxl_wnd_array) asm("_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE");

void fh_destroypixelwands(Value* pxl_wnd_array) asm("_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE");

bool fh_isdrawingwand(TypedValue* var) asm("_ZN4HPHP15f_isdrawingwandERKNS_7VariantE");

bool fh_ismagickwand(TypedValue* var) asm("_ZN4HPHP14f_ismagickwandERKNS_7VariantE");

bool fh_ispixeliterator(TypedValue* var) asm("_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE");

bool fh_ispixelwand(TypedValue* var) asm("_ZN4HPHP13f_ispixelwandERKNS_7VariantE");

void fh_cleardrawingwand(Value* drw_wnd) asm("_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE");

void fh_clearmagickwand(Value* mgck_wnd) asm("_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE");

void fh_clearpixeliterator(Value* pxl_iter) asm("_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE");

void fh_clearpixelwand(Value* pxl_wnd) asm("_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE");

Value* fh_clonedrawingwand(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE");

Value* fh_clonemagickwand(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE");

Value* fh_wandgetexception(Value* _rv, Value* wnd) asm("_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE");

Value* fh_wandgetexceptionstring(Value* _rv, Value* wnd) asm("_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE");

long fh_wandgetexceptiontype(Value* wnd) asm("_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE");

bool fh_wandhasexception(Value* wnd) asm("_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE");

void fh_drawaffine(Value* drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) asm("_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd");

void fh_drawannotation(Value* drw_wnd, double x, double y, Value* text) asm("_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE");

void fh_drawarc(Value* drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) asm("_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd");

void fh_drawbezier(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE");

void fh_drawcircle(Value* drw_wnd, double ox, double oy, double px, double py) asm("_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd");

void fh_drawcolor(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi");

void fh_drawcomment(Value* drw_wnd, Value* comment) asm("_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE");

bool fh_drawcomposite(Value* drw_wnd, int composite_operator, double x, double y, double width, double height, Value* mgck_wnd) asm("_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_");

void fh_drawellipse(Value* drw_wnd, double ox, double oy, double rx, double ry, double start, double end) asm("_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd");

Value* fh_drawgetclippath(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE");

long fh_drawgetcliprule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE");

long fh_drawgetclipunits(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE");

Value* fh_drawgetexception(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE");

Value* fh_drawgetexceptionstring(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE");

long fh_drawgetexceptiontype(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE");

double fh_drawgetfillalpha(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE");

Value* fh_drawgetfillcolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE");

double fh_drawgetfillopacity(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE");

long fh_drawgetfillrule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE");

Value* fh_drawgetfont(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP13f_drawgetfontERKNS_6ObjectE");

Value* fh_drawgetfontfamily(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE");

double fh_drawgetfontsize(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE");

long fh_drawgetfontstretch(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE");

long fh_drawgetfontstyle(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE");

double fh_drawgetfontweight(Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE");

long fh_drawgetgravity(Value* drw_wnd) asm("_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE");

double fh_drawgetstrokealpha(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE");

bool fh_drawgetstrokeantialias(Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE");

Value* fh_drawgetstrokecolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE");

Value* fh_drawgetstrokedasharray(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE");

double fh_drawgetstrokedashoffset(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE");

long fh_drawgetstrokelinecap(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE");

long fh_drawgetstrokelinejoin(Value* drw_wnd) asm("_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE");

double fh_drawgetstrokemiterlimit(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE");

double fh_drawgetstrokeopacity(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE");

double fh_drawgetstrokewidth(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE");

long fh_drawgettextalignment(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE");

bool fh_drawgettextantialias(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE");

long fh_drawgettextdecoration(Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE");

Value* fh_drawgettextencoding(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE");

Value* fh_drawgettextundercolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE");

Value* fh_drawgetvectorgraphics(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE");

void fh_drawline(Value* drw_wnd, double sx, double sy, double ex, double ey) asm("_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd");

void fh_drawmatte(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi");

void fh_drawpathclose(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE");

void fh_drawpathcurvetoabsolute(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd");

void fh_drawpathcurvetoquadraticbezierabsolute(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd");

void fh_drawpathcurvetoquadraticbezierrelative(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd");

void fh_drawpathcurvetoquadraticbeziersmoothabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd");

void fh_drawpathcurvetoquadraticbeziersmoothrelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd");

void fh_drawpathcurvetorelative(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd");

void fh_drawpathcurvetosmoothabsolute(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd");

void fh_drawpathcurvetosmoothrelative(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd");

void fh_drawpathellipticarcabsolute(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd");

void fh_drawpathellipticarcrelative(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd");

void fh_drawpathfinish(Value* drw_wnd) asm("_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE");

void fh_drawpathlinetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd");

void fh_drawpathlinetohorizontalabsolute(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd");

void fh_drawpathlinetohorizontalrelative(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd");

void fh_drawpathlinetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd");

void fh_drawpathlinetoverticalabsolute(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd");

void fh_drawpathlinetoverticalrelative(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd");

void fh_drawpathmovetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd");

void fh_drawpathmovetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd");

void fh_drawpathstart(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathstartERKNS_6ObjectE");

void fh_drawpoint(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawpointERKNS_6ObjectEdd");

void fh_drawpolygon(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE");

void fh_drawpolyline(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE");

void fh_drawrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd");

bool fh_drawrender(Value* drw_wnd) asm("_ZN4HPHP12f_drawrenderERKNS_6ObjectE");

void fh_drawrotate(Value* drw_wnd, double degrees) asm("_ZN4HPHP12f_drawrotateERKNS_6ObjectEd");

void fh_drawroundrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) asm("_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd");

void fh_drawscale(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd");

bool fh_drawsetclippath(Value* drw_wnd, Value* clip_path) asm("_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE");

void fh_drawsetcliprule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi");

void fh_drawsetclipunits(Value* drw_wnd, int clip_path_units) asm("_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi");

void fh_drawsetfillalpha(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd");

void fh_drawsetfillcolor(Value* drw_wnd, Value* fill_pxl_wnd) asm("_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_");

void fh_drawsetfillopacity(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd");

bool fh_drawsetfillpatternurl(Value* drw_wnd, Value* fill_url) asm("_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE");

void fh_drawsetfillrule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi");

bool fh_drawsetfont(Value* drw_wnd, Value* font_file) asm("_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE");

bool fh_drawsetfontfamily(Value* drw_wnd, Value* font_family) asm("_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE");

void fh_drawsetfontsize(Value* drw_wnd, double pointsize) asm("_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd");

void fh_drawsetfontstretch(Value* drw_wnd, int stretch_type) asm("_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi");

void fh_drawsetfontstyle(Value* drw_wnd, int style_type) asm("_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi");

void fh_drawsetfontweight(Value* drw_wnd, double font_weight) asm("_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd");

void fh_drawsetgravity(Value* drw_wnd, int gravity_type) asm("_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi");

void fh_drawsetstrokealpha(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd");

void fh_drawsetstrokeantialias(Value* drw_wnd, bool stroke_antialias) asm("_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb");

void fh_drawsetstrokecolor(Value* drw_wnd, Value* strokecolor_pxl_wnd) asm("_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_");

void fh_drawsetstrokedasharray(Value* drw_wnd, Value* dash_array) asm("_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE");

void fh_drawsetstrokedashoffset(Value* drw_wnd, double dash_offset) asm("_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd");

void fh_drawsetstrokelinecap(Value* drw_wnd, int line_cap) asm("_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi");

void fh_drawsetstrokelinejoin(Value* drw_wnd, int line_join) asm("_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi");

void fh_drawsetstrokemiterlimit(Value* drw_wnd, double miterlimit) asm("_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd");

void fh_drawsetstrokeopacity(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd");

bool fh_drawsetstrokepatternurl(Value* drw_wnd, Value* stroke_url) asm("_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE");

void fh_drawsetstrokewidth(Value* drw_wnd, double stroke_width) asm("_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd");

void fh_drawsettextalignment(Value* drw_wnd, int align_type) asm("_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi");

void fh_drawsettextantialias(Value* drw_wnd, bool text_antialias) asm("_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb");

void fh_drawsettextdecoration(Value* drw_wnd, int decoration_type) asm("_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi");

void fh_drawsettextencoding(Value* drw_wnd, Value* encoding) asm("_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE");

void fh_drawsettextundercolor(Value* drw_wnd, Value* undercolor_pxl_wnd) asm("_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_");

bool fh_drawsetvectorgraphics(Value* drw_wnd, Value* vector_graphics) asm("_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE");

void fh_drawsetviewbox(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd");

void fh_drawskewx(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewxERKNS_6ObjectEd");

void fh_drawskewy(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewyERKNS_6ObjectEd");

void fh_drawtranslate(Value* drw_wnd, double x, double y) asm("_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd");

void fh_pushdrawingwand(Value* drw_wnd) asm("_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE");

void fh_drawpushclippath(Value* drw_wnd, Value* clip_path_id) asm("_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE");

void fh_drawpushdefs(Value* drw_wnd) asm("_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE");

void fh_drawpushpattern(Value* drw_wnd, Value* pattern_id, double x, double y, double width, double height) asm("_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd");

void fh_popdrawingwand(Value* drw_wnd) asm("_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE");

void fh_drawpopclippath(Value* drw_wnd) asm("_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE");

void fh_drawpopdefs(Value* drw_wnd) asm("_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE");

void fh_drawpoppattern(Value* drw_wnd) asm("_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE");

bool fh_magickadaptivethresholdimage(Value* mgck_wnd, double width, double height, double offset) asm("_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd");

bool fh_magickaddimage(Value* mgck_wnd, Value* add_wand) asm("_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_");

bool fh_magickaddnoiseimage(Value* mgck_wnd, int noise_type) asm("_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi");

bool fh_magickaffinetransformimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_");

bool fh_magickannotateimage(Value* mgck_wnd, Value* drw_wnd, double x, double y, double angle, Value* text) asm("_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE");

Value* fh_magickappendimages(Value* _rv, Value* mgck_wnd, bool stack_vertical) asm("_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb");

Value* fh_magickaverageimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE");

bool fh_magickblackthresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_");

bool fh_magickblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi");

bool fh_magickborderimage(Value* mgck_wnd, Value* bordercolor, double width, double height) asm("_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd");

bool fh_magickcharcoalimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd");

bool fh_magickchopimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii");

bool fh_magickclipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickclipimageERKNS_6ObjectE");

bool fh_magickclippathimage(Value* mgck_wnd, Value* pathname, bool inside) asm("_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb");

Value* fh_magickcoalesceimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE");

bool fh_magickcolorfloodfillimage(Value* mgck_wnd, Value* fillcolor_pxl_wnd, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii");

bool fh_magickcolorizeimage(Value* mgck_wnd, Value* colorize, Value* opacity_pxl_wnd) asm("_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_");

Value* fh_magickcombineimages(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi");

bool fh_magickcommentimage(Value* mgck_wnd, Value* comment) asm("_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE");

Value* fh_magickcompareimages(Value* _rv, Value* mgck_wnd, Value* reference_wnd, int metric_type, int channel_type) asm("_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii");

bool fh_magickcompositeimage(Value* mgck_wnd, Value* composite_wnd, int composite_operator, int x, int y) asm("_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii");

bool fh_magickconstituteimage(Value* mgck_wnd, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE");

bool fh_magickcontrastimage(Value* mgck_wnd, bool sharpen) asm("_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb");

bool fh_magickconvolveimage(Value* mgck_wnd, Value* kernel_array, int channel_type) asm("_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi");

bool fh_magickcropimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii");

bool fh_magickcyclecolormapimage(Value* mgck_wnd, int num_positions) asm("_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi");

Value* fh_magickdeconstructimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE");

Value* fh_magickdescribeimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE");

bool fh_magickdespeckleimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE");

bool fh_magickdrawimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_");

bool fh_magickechoimageblob(Value* mgck_wnd) asm("_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE");

bool fh_magickechoimagesblob(Value* mgck_wnd) asm("_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE");

bool fh_magickedgeimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd");

bool fh_magickembossimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd");

bool fh_magickenhanceimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE");

bool fh_magickequalizeimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE");

bool fh_magickevaluateimage(Value* mgck_wnd, int evaluate_op, double constant, int channel_type) asm("_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi");

Value* fh_magickflattenimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE");

bool fh_magickflipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflipimageERKNS_6ObjectE");

bool fh_magickflopimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflopimageERKNS_6ObjectE");

bool fh_magickframeimage(Value* mgck_wnd, Value* matte_color, double width, double height, int inner_bevel, int outer_bevel) asm("_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii");

Value* fh_magickfximage(Value* _rv, Value* mgck_wnd, Value* expression, int channel_type) asm("_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi");

bool fh_magickgammaimage(Value* mgck_wnd, double gamma, int channel_type) asm("_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi");

bool fh_magickgaussianblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi");

double fh_magickgetcharheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb");

double fh_magickgetcharwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb");

Value* fh_magickgetexception(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE");

Value* fh_magickgetexceptionstring(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE");

long fh_magickgetexceptiontype(Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE");

Value* fh_magickgetfilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE");

Value* fh_magickgetformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_magickgetformatERKNS_6ObjectE");

Value* fh_magickgetimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP16f_magickgetimageERKNS_6ObjectE");

Value* fh_magickgetimagebackgroundcolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE");

Value* fh_magickgetimageblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE");

Value* fh_magickgetimageblueprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE");

Value* fh_magickgetimagebordercolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE");

Value* fh_magickgetimagechannelmean(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi");

Value* fh_magickgetimagecolormapcolor(Value* _rv, Value* mgck_wnd, double index) asm("_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd");

double fh_magickgetimagecolors(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE");

long fh_magickgetimagecolorspace(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE");

long fh_magickgetimagecompose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE");

long fh_magickgetimagecompression(Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE");

double fh_magickgetimagecompressionquality(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE");

double fh_magickgetimagedelay(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE");

double fh_magickgetimagedepth(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi");

long fh_magickgetimagedispose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE");

Value* fh_magickgetimageextrema(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi");

Value* fh_magickgetimagefilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE");

Value* fh_magickgetimageformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE");

double fh_magickgetimagegamma(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE");

Value* fh_magickgetimagegreenprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE");

double fh_magickgetimageheight(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE");

Value* fh_magickgetimagehistogram(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE");

long fh_magickgetimageindex(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE");

long fh_magickgetimageinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE");

double fh_magickgetimageiterations(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE");

Value* fh_magickgetimagemattecolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE");

Value* fh_magickgetimagemimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE");

Value* fh_magickgetimagepixels(Value* _rv, Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type) asm("_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi");

Value* fh_magickgetimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE");

Value* fh_magickgetimageredprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE");

long fh_magickgetimagerenderingintent(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE");

Value* fh_magickgetimageresolution(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE");

double fh_magickgetimagescene(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE");

Value* fh_magickgetimagesignature(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE");

long fh_magickgetimagesize(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE");

long fh_magickgetimagetype(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE");

long fh_magickgetimageunits(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE");

long fh_magickgetimagevirtualpixelmethod(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE");

Value* fh_magickgetimagewhitepoint(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE");

double fh_magickgetimagewidth(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE");

Value* fh_magickgetimagesblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE");

long fh_magickgetinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE");

double fh_magickgetmaxtextadvance(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb");

Value* fh_magickgetmimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE");

double fh_magickgetnumberimages(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE");

Value* fh_magickgetsamplingfactors(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE");

Value* fh_magickgetsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE");

double fh_magickgetstringheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb");

double fh_magickgetstringwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb");

double fh_magickgettextascent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb");

double fh_magickgettextdescent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb");

Value* fh_magickgetwandsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE");

bool fh_magickhasnextimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE");

bool fh_magickhaspreviousimage(Value* mgck_wnd) asm("_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE");

bool fh_magickimplodeimage(Value* mgck_wnd, double amount) asm("_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd");

bool fh_magicklabelimage(Value* mgck_wnd, Value* label) asm("_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE");

bool fh_magicklevelimage(Value* mgck_wnd, double black_point, double gamma, double white_point, int channel_type) asm("_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi");

bool fh_magickmagnifyimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE");

bool fh_magickmapimage(Value* mgck_wnd, Value* map_wand, bool dither) asm("_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b");

bool fh_magickmattefloodfillimage(Value* mgck_wnd, double opacity, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii");

bool fh_magickmedianfilterimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd");

bool fh_magickminifyimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE");

bool fh_magickmodulateimage(Value* mgck_wnd, double brightness, double saturation, double hue) asm("_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd");

Value* fh_magickmontageimage(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* tile_geometry, Value* thumbnail_geometry, int montage_mode, Value* frame) asm("_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_");

Value* fh_magickmorphimages(Value* _rv, Value* mgck_wnd, double number_frames) asm("_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd");

Value* fh_magickmosaicimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE");

bool fh_magickmotionblurimage(Value* mgck_wnd, double radius, double sigma, double angle) asm("_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd");

bool fh_magicknegateimage(Value* mgck_wnd, bool only_the_gray, int channel_type) asm("_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi");

bool fh_magicknewimage(Value* mgck_wnd, double width, double height, Value* imagemagick_col_str) asm("_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE");

bool fh_magicknextimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magicknextimageERKNS_6ObjectE");

bool fh_magicknormalizeimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE");

bool fh_magickoilpaintimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd");

bool fh_magickpaintopaqueimage(Value* mgck_wnd, Value* target_pxl_wnd, Value* fill_pxl_wnd, double fuzz) asm("_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d");

bool fh_magickpainttransparentimage(Value* mgck_wnd, Value* target, double opacity, double fuzz) asm("_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd");

bool fh_magickpingimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE");

bool fh_magickposterizeimage(Value* mgck_wnd, double levels, bool dither) asm("_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb");

Value* fh_magickpreviewimages(Value* _rv, Value* mgck_wnd, int preview) asm("_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi");

bool fh_magickpreviousimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE");

bool fh_magickprofileimage(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_");

bool fh_magickquantizeimage(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb");

bool fh_magickquantizeimages(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb");

Value* fh_magickqueryfontmetrics(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb");

bool fh_magickradialblurimage(Value* mgck_wnd, double angle) asm("_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd");

bool fh_magickraiseimage(Value* mgck_wnd, double width, double height, int x, int y, bool raise) asm("_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib");

bool fh_magickreadimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE");

bool fh_magickreadimageblob(Value* mgck_wnd, Value* blob) asm("_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE");

bool fh_magickreadimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_");

bool fh_magickreadimages(Value* mgck_wnd, Value* img_filenames_array) asm("_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE");

bool fh_magickreducenoiseimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd");

bool fh_magickremoveimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE");

Value* fh_magickremoveimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE");

bool fh_magickremoveimageprofiles(Value* mgck_wnd) asm("_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE");

bool fh_magickresampleimage(Value* mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) asm("_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid");

void fh_magickresetiterator(Value* mgck_wnd) asm("_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE");

bool fh_magickresizeimage(Value* mgck_wnd, double columns, double rows, int filter_type, double blur) asm("_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid");

bool fh_magickrollimage(Value* mgck_wnd, int x_offset, int y_offset) asm("_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii");

bool fh_magickrotateimage(Value* mgck_wnd, Value* background, double degrees) asm("_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d");

bool fh_magicksampleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd");

bool fh_magickscaleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd");

bool fh_magickseparateimagechannel(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi");

bool fh_magicksetcompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd");

bool fh_magicksetfilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE");

void fh_magicksetfirstiterator(Value* mgck_wnd) asm("_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE");

bool fh_magicksetformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE");

bool fh_magicksetimage(Value* mgck_wnd, Value* replace_wand) asm("_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_");

bool fh_magicksetimagebackgroundcolor(Value* mgck_wnd, Value* background_pxl_wnd) asm("_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_");

bool fh_magicksetimagebias(Value* mgck_wnd, double bias) asm("_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd");

bool fh_magicksetimageblueprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd");

bool fh_magicksetimagebordercolor(Value* mgck_wnd, Value* border_pxl_wnd) asm("_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_");

bool fh_magicksetimagecolormapcolor(Value* mgck_wnd, double index, Value* mapcolor_pxl_wnd) asm("_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_");

bool fh_magicksetimagecolorspace(Value* mgck_wnd, int colorspace_type) asm("_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi");

bool fh_magicksetimagecompose(Value* mgck_wnd, int composite_operator) asm("_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi");

bool fh_magicksetimagecompression(Value* mgck_wnd, int compression_type) asm("_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi");

bool fh_magicksetimagecompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd");

bool fh_magicksetimagedelay(Value* mgck_wnd, double delay) asm("_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd");

bool fh_magicksetimagedepth(Value* mgck_wnd, int depth, int channel_type) asm("_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii");

bool fh_magicksetimagedispose(Value* mgck_wnd, int dispose_type) asm("_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi");

bool fh_magicksetimagefilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE");

bool fh_magicksetimageformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE");

bool fh_magicksetimagegamma(Value* mgck_wnd, double gamma) asm("_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd");

bool fh_magicksetimagegreenprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd");

bool fh_magicksetimageindex(Value* mgck_wnd, int index) asm("_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi");

bool fh_magicksetimageinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi");

bool fh_magicksetimageiterations(Value* mgck_wnd, double iterations) asm("_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd");

bool fh_magicksetimagemattecolor(Value* mgck_wnd, Value* matte_pxl_wnd) asm("_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_");

bool fh_magicksetimageoption(Value* mgck_wnd, Value* format, Value* key, Value* value) asm("_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_");

bool fh_magicksetimagepixels(Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE");

bool fh_magicksetimageprofile(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_");

bool fh_magicksetimageredprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd");

bool fh_magicksetimagerenderingintent(Value* mgck_wnd, int rendering_intent) asm("_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi");

bool fh_magicksetimageresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd");

bool fh_magicksetimagescene(Value* mgck_wnd, double scene) asm("_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd");

bool fh_magicksetimagetype(Value* mgck_wnd, int image_type) asm("_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi");

bool fh_magicksetimageunits(Value* mgck_wnd, int resolution_type) asm("_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi");

bool fh_magicksetimagevirtualpixelmethod(Value* mgck_wnd, int virtual_pixel_method) asm("_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi");

bool fh_magicksetimagewhitepoint(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd");

bool fh_magicksetinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi");

void fh_magicksetlastiterator(Value* mgck_wnd) asm("_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE");

bool fh_magicksetpassphrase(Value* mgck_wnd, Value* passphrase) asm("_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE");

bool fh_magicksetresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd");

bool fh_magicksetsamplingfactors(Value* mgck_wnd, double number_factors, Value* sampling_factors) asm("_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE");

bool fh_magicksetsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii");

bool fh_magicksetwandsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii");

bool fh_magicksharpenimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi");

bool fh_magickshaveimage(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii");

bool fh_magickshearimage(Value* mgck_wnd, Value* background, double x_shear, double y_shear) asm("_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd");

bool fh_magicksolarizeimage(Value* mgck_wnd, double threshold) asm("_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd");

bool fh_magickspliceimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii");

bool fh_magickspreadimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd");

Value* fh_magicksteganoimage(Value* _rv, Value* mgck_wnd, Value* watermark_wand, int offset) asm("_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i");

bool fh_magickstereoimage(Value* mgck_wnd, Value* offset_wand) asm("_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_");

bool fh_magickstripimage(Value* mgck_wnd) asm("_ZN4HPHP18f_magickstripimageERKNS_6ObjectE");

bool fh_magickswirlimage(Value* mgck_wnd, double degrees) asm("_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd");

Value* fh_magicktextureimage(Value* _rv, Value* mgck_wnd, Value* texture_wand) asm("_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_");

bool fh_magickthresholdimage(Value* mgck_wnd, double threshold, int channel_type) asm("_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi");

bool fh_magicktintimage(Value* mgck_wnd, int tint_pxl_wnd, Value* opacity_pxl_wnd) asm("_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_");

Value* fh_magicktransformimage(Value* _rv, Value* mgck_wnd, Value* crop, Value* geometry) asm("_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_");

bool fh_magicktrimimage(Value* mgck_wnd, double fuzz) asm("_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd");

bool fh_magickunsharpmaskimage(Value* mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type) asm("_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi");

bool fh_magickwaveimage(Value* mgck_wnd, double amplitude, double wave_length) asm("_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd");

bool fh_magickwhitethresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_");

bool fh_magickwriteimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE");

bool fh_magickwriteimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_");

bool fh_magickwriteimages(Value* mgck_wnd, Value* filename, bool join_images) asm("_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb");

bool fh_magickwriteimagesfile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_");

double fh_pixelgetalpha(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE");

double fh_pixelgetalphaquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE");

double fh_pixelgetblack(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE");

double fh_pixelgetblackquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE");

double fh_pixelgetblue(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE");

double fh_pixelgetbluequantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE");

Value* fh_pixelgetcolorasstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE");

double fh_pixelgetcolorcount(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE");

double fh_pixelgetcyan(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE");

double fh_pixelgetcyanquantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE");

Value* fh_pixelgetexception(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE");

Value* fh_pixelgetexceptionstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE");

long fh_pixelgetexceptiontype(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE");

double fh_pixelgetgreen(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE");

double fh_pixelgetgreenquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE");

double fh_pixelgetindex(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE");

double fh_pixelgetmagenta(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE");

double fh_pixelgetmagentaquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE");

double fh_pixelgetopacity(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE");

double fh_pixelgetopacityquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE");

Value* fh_pixelgetquantumcolor(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE");

double fh_pixelgetred(Value* pxl_wnd) asm("_ZN4HPHP13f_pixelgetredERKNS_6ObjectE");

double fh_pixelgetredquantum(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE");

double fh_pixelgetyellow(Value* pxl_wnd) asm("_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE");

double fh_pixelgetyellowquantum(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE");

void fh_pixelsetalpha(Value* pxl_wnd, double alpha) asm("_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd");

void fh_pixelsetalphaquantum(Value* pxl_wnd, double alpha) asm("_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd");

void fh_pixelsetblack(Value* pxl_wnd, double black) asm("_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd");

void fh_pixelsetblackquantum(Value* pxl_wnd, double black) asm("_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd");

void fh_pixelsetblue(Value* pxl_wnd, double blue) asm("_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd");

void fh_pixelsetbluequantum(Value* pxl_wnd, double blue) asm("_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd");

void fh_pixelsetcolor(Value* pxl_wnd, Value* imagemagick_col_str) asm("_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE");

void fh_pixelsetcolorcount(Value* pxl_wnd, int count) asm("_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi");

void fh_pixelsetcyan(Value* pxl_wnd, double cyan) asm("_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd");

void fh_pixelsetcyanquantum(Value* pxl_wnd, double cyan) asm("_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd");

void fh_pixelsetgreen(Value* pxl_wnd, double green) asm("_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd");

void fh_pixelsetgreenquantum(Value* pxl_wnd, double green) asm("_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd");

void fh_pixelsetindex(Value* pxl_wnd, double index) asm("_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd");

void fh_pixelsetmagenta(Value* pxl_wnd, double magenta) asm("_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd");

void fh_pixelsetmagentaquantum(Value* pxl_wnd, double magenta) asm("_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd");

void fh_pixelsetopacity(Value* pxl_wnd, double opacity) asm("_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd");

void fh_pixelsetopacityquantum(Value* pxl_wnd, double opacity) asm("_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd");

void fh_pixelsetquantumcolor(Value* pxl_wnd, double red, double green, double blue, double opacity) asm("_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd");

void fh_pixelsetred(Value* pxl_wnd, double red) asm("_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd");

void fh_pixelsetredquantum(Value* pxl_wnd, double red) asm("_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd");

void fh_pixelsetyellow(Value* pxl_wnd, double yellow) asm("_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd");

void fh_pixelsetyellowquantum(Value* pxl_wnd, double yellow) asm("_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd");

Value* fh_pixelgetiteratorexception(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE");

Value* fh_pixelgetiteratorexceptionstring(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE");

long fh_pixelgetiteratorexceptiontype(Value* pxl_iter) asm("_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE");

Value* fh_pixelgetnextiteratorrow(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE");

void fh_pixelresetiterator(Value* pxl_iter) asm("_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE");

bool fh_pixelsetiteratorrow(Value* pxl_iter, int row) asm("_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi");

bool fh_pixelsynciterator(Value* pxl_iter) asm("_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE");

} // namespace HPHP
