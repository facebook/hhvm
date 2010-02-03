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

#include <cpp/ext/ext_magick.h>
#include "crutch.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String f_magickgetcopyright() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetcopyright", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgethomeurl() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgethomeurl", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetpackagename() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetpackagename", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetquantumdepth() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetquantumdepth", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetreleasedate() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetreleasedate", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetresourcelimit(int resource_type) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(resource_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetresourcelimit", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetversion() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetversion", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetversionnumber() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetversionnumber", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetversionstring() {
  Array _schema((ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetversionstring", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickqueryconfigureoption(CStrRef option) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(option), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickqueryconfigureoption", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickqueryconfigureoptions(CStrRef pattern) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(pattern), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickqueryconfigureoptions", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickqueryfonts(CStrRef pattern) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(pattern), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickqueryfonts", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickqueryformats(CStrRef pattern) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(pattern), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickqueryformats", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetresourcelimit(int resource_type, double limit) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(resource_type), NEW(ArrayElement)(limit), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetresourcelimit", _schema, _params);
  return (Variant)_ret[0];
}

Object f_newdrawingwand() {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newdrawingwand", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_newmagickwand() {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newmagickwand", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_newpixeliterator(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newpixeliterator", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_newpixelregioniterator(CObjRef mgck_wnd, int x, int y, int columns, int rows) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newpixelregioniterator", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_newpixelwand(CStrRef imagemagick_col_str /* = null_string */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(imagemagick_col_str), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newpixelwand", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_newpixelwandarray(int num_pxl_wnds) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(num_pxl_wnds), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newpixelwandarray", _schema, _params);
  return (Variant)_ret[0];
}

Array f_newpixelwands(int num_pxl_wnds) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(num_pxl_wnds), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("newpixelwands", _schema, _params);
  return (Variant)_ret[0];
}

void f_destroydrawingwand(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("destroydrawingwand", _schema, _params);
}

void f_destroymagickwand(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("destroymagickwand", _schema, _params);
}

void f_destroypixeliterator(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Crutch::Invoke("destroypixeliterator", _schema, _params);
}

void f_destroypixelwand(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("destroypixelwand", _schema, _params);
}

void f_destroypixelwandarray(CArrRef pxl_wnd_array) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(pxl_wnd_array), (ArrayElement*)NULL);
  Crutch::Invoke("destroypixelwandarray", _schema, _params);
}

void f_destroypixelwands(CArrRef pxl_wnd_array) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(pxl_wnd_array), (ArrayElement*)NULL);
  Crutch::Invoke("destroypixelwands", _schema, _params);
}

bool f_isdrawingwand(CVarRef var) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(var), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("isdrawingwand", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ismagickwand(CVarRef var) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(var), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ismagickwand", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ispixeliterator(CVarRef var) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(var), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ispixeliterator", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ispixelwand(CVarRef var) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(var), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ispixelwand", _schema, _params);
  return (Variant)_ret[0];
}

void f_cleardrawingwand(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("cleardrawingwand", _schema, _params);
}

void f_clearmagickwand(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("clearmagickwand", _schema, _params);
}

void f_clearpixeliterator(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Crutch::Invoke("clearpixeliterator", _schema, _params);
}

void f_clearpixelwand(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("clearpixelwand", _schema, _params);
}

Object f_clonedrawingwand(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("clonedrawingwand", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_clonemagickwand(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("clonemagickwand", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_wandgetexception(CObjRef wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("wandgetexception", _schema, _params);
  return (Variant)_ret[0];
}

String f_wandgetexceptionstring(CObjRef wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("wandgetexceptionstring", _schema, _params);
  return (Variant)_ret[0];
}

int f_wandgetexceptiontype(CObjRef wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("wandgetexceptiontype", _schema, _params);
  return (Variant)_ret[0];
}

bool f_wandhasexception(CObjRef wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("wandhasexception", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawaffine(CObjRef drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(sx), NEW(ArrayElement)(sy), NEW(ArrayElement)(rx), NEW(ArrayElement)(ry), NEW(ArrayElement)(tx), NEW(ArrayElement)(ty), (ArrayElement*)NULL);
  Crutch::Invoke("drawaffine", _schema, _params);
}

void f_drawannotation(CObjRef drw_wnd, double x, double y, CStrRef text) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(text), (ArrayElement*)NULL);
  Crutch::Invoke("drawannotation", _schema, _params);
}

void f_drawarc(CObjRef drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(sx), NEW(ArrayElement)(sy), NEW(ArrayElement)(ex), NEW(ArrayElement)(ey), NEW(ArrayElement)(sd), NEW(ArrayElement)(ed), (ArrayElement*)NULL);
  Crutch::Invoke("drawarc", _schema, _params);
}

void f_drawbezier(CObjRef drw_wnd, CArrRef x_y_points_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x_y_points_array), (ArrayElement*)NULL);
  Crutch::Invoke("drawbezier", _schema, _params);
}

void f_drawcircle(CObjRef drw_wnd, double ox, double oy, double px, double py) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(ox), NEW(ArrayElement)(oy), NEW(ArrayElement)(px), NEW(ArrayElement)(py), (ArrayElement*)NULL);
  Crutch::Invoke("drawcircle", _schema, _params);
}

void f_drawcolor(CObjRef drw_wnd, double x, double y, int paint_method) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(paint_method), (ArrayElement*)NULL);
  Crutch::Invoke("drawcolor", _schema, _params);
}

void f_drawcomment(CObjRef drw_wnd, CStrRef comment) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(comment), (ArrayElement*)NULL);
  Crutch::Invoke("drawcomment", _schema, _params);
}

bool f_drawcomposite(CObjRef drw_wnd, int composite_operator, double x, double y, double width, double height, CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(6, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(composite_operator), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawcomposite", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawellipse(CObjRef drw_wnd, double ox, double oy, double rx, double ry, double start, double end) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(ox), NEW(ArrayElement)(oy), NEW(ArrayElement)(rx), NEW(ArrayElement)(ry), NEW(ArrayElement)(start), NEW(ArrayElement)(end), (ArrayElement*)NULL);
  Crutch::Invoke("drawellipse", _schema, _params);
}

String f_drawgetclippath(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetclippath", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetcliprule(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetcliprule", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetclipunits(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetclipunits", _schema, _params);
  return (Variant)_ret[0];
}

Array f_drawgetexception(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetexception", _schema, _params);
  return (Variant)_ret[0];
}

String f_drawgetexceptionstring(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetexceptionstring", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetexceptiontype(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetexceptiontype", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetfillalpha(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfillalpha", _schema, _params);
  return (Variant)_ret[0];
}

Object f_drawgetfillcolor(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfillcolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

double f_drawgetfillopacity(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfillopacity", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetfillrule(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfillrule", _schema, _params);
  return (Variant)_ret[0];
}

String f_drawgetfont(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfont", _schema, _params);
  return (Variant)_ret[0];
}

String f_drawgetfontfamily(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfontfamily", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetfontsize(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfontsize", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetfontstretch(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfontstretch", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetfontstyle(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfontstyle", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetfontweight(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetfontweight", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetgravity(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetgravity", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetstrokealpha(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokealpha", _schema, _params);
  return (Variant)_ret[0];
}

bool f_drawgetstrokeantialias(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokeantialias", _schema, _params);
  return (Variant)_ret[0];
}

Object f_drawgetstrokecolor(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokecolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_drawgetstrokedasharray(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokedasharray", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetstrokedashoffset(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokedashoffset", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetstrokelinecap(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokelinecap", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgetstrokelinejoin(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokelinejoin", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetstrokemiterlimit(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokemiterlimit", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetstrokeopacity(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokeopacity", _schema, _params);
  return (Variant)_ret[0];
}

double f_drawgetstrokewidth(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetstrokewidth", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgettextalignment(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgettextalignment", _schema, _params);
  return (Variant)_ret[0];
}

bool f_drawgettextantialias(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgettextantialias", _schema, _params);
  return (Variant)_ret[0];
}

int f_drawgettextdecoration(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgettextdecoration", _schema, _params);
  return (Variant)_ret[0];
}

String f_drawgettextencoding(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgettextencoding", _schema, _params);
  return (Variant)_ret[0];
}

Object f_drawgettextundercolor(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgettextundercolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_drawgetvectorgraphics(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawgetvectorgraphics", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawline(CObjRef drw_wnd, double sx, double sy, double ex, double ey) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(sx), NEW(ArrayElement)(sy), NEW(ArrayElement)(ex), NEW(ArrayElement)(ey), (ArrayElement*)NULL);
  Crutch::Invoke("drawline", _schema, _params);
}

void f_drawmatte(CObjRef drw_wnd, double x, double y, int paint_method) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(paint_method), (ArrayElement*)NULL);
  Crutch::Invoke("drawmatte", _schema, _params);
}

void f_drawpathclose(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathclose", _schema, _params);
}

void f_drawpathcurvetoabsolute(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetoabsolute", _schema, _params);
}

void f_drawpathcurvetoquadraticbezierabsolute(CObjRef drw_wnd, double x1, double y1, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetoquadraticbezierabsolute", _schema, _params);
}

void f_drawpathcurvetoquadraticbezierrelative(CObjRef drw_wnd, double x1, double y1, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetoquadraticbezierrelative", _schema, _params);
}

void f_drawpathcurvetoquadraticbeziersmoothabsolute(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetoquadraticbeziersmoothabsolute", _schema, _params);
}

void f_drawpathcurvetoquadraticbeziersmoothrelative(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetoquadraticbeziersmoothrelative", _schema, _params);
}

void f_drawpathcurvetorelative(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetorelative", _schema, _params);
}

void f_drawpathcurvetosmoothabsolute(CObjRef drw_wnd, double x2, double y2, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetosmoothabsolute", _schema, _params);
}

void f_drawpathcurvetosmoothrelative(CObjRef drw_wnd, double x2, double y2, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathcurvetosmoothrelative", _schema, _params);
}

void f_drawpathellipticarcabsolute(CObjRef drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(rx), NEW(ArrayElement)(ry), NEW(ArrayElement)(x_axis_rotation), NEW(ArrayElement)(large_arc_flag), NEW(ArrayElement)(sweep_flag), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathellipticarcabsolute", _schema, _params);
}

void f_drawpathellipticarcrelative(CObjRef drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(rx), NEW(ArrayElement)(ry), NEW(ArrayElement)(x_axis_rotation), NEW(ArrayElement)(large_arc_flag), NEW(ArrayElement)(sweep_flag), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathellipticarcrelative", _schema, _params);
}

void f_drawpathfinish(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathfinish", _schema, _params);
}

void f_drawpathlinetoabsolute(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetoabsolute", _schema, _params);
}

void f_drawpathlinetohorizontalabsolute(CObjRef drw_wnd, double x) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetohorizontalabsolute", _schema, _params);
}

void f_drawpathlinetohorizontalrelative(CObjRef drw_wnd, double x) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetohorizontalrelative", _schema, _params);
}

void f_drawpathlinetorelative(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetorelative", _schema, _params);
}

void f_drawpathlinetoverticalabsolute(CObjRef drw_wnd, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetoverticalabsolute", _schema, _params);
}

void f_drawpathlinetoverticalrelative(CObjRef drw_wnd, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathlinetoverticalrelative", _schema, _params);
}

void f_drawpathmovetoabsolute(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathmovetoabsolute", _schema, _params);
}

void f_drawpathmovetorelative(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathmovetorelative", _schema, _params);
}

void f_drawpathstart(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpathstart", _schema, _params);
}

void f_drawpoint(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawpoint", _schema, _params);
}

void f_drawpolygon(CObjRef drw_wnd, CArrRef x_y_points_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x_y_points_array), (ArrayElement*)NULL);
  Crutch::Invoke("drawpolygon", _schema, _params);
}

void f_drawpolyline(CObjRef drw_wnd, CArrRef x_y_points_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x_y_points_array), (ArrayElement*)NULL);
  Crutch::Invoke("drawpolyline", _schema, _params);
}

void f_drawrectangle(CObjRef drw_wnd, double x1, double y1, double x2, double y2) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), (ArrayElement*)NULL);
  Crutch::Invoke("drawrectangle", _schema, _params);
}

bool f_drawrender(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawrender", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawrotate(CObjRef drw_wnd, double degrees) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(degrees), (ArrayElement*)NULL);
  Crutch::Invoke("drawrotate", _schema, _params);
}

void f_drawroundrectangle(CObjRef drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), NEW(ArrayElement)(rx), NEW(ArrayElement)(ry), (ArrayElement*)NULL);
  Crutch::Invoke("drawroundrectangle", _schema, _params);
}

void f_drawscale(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawscale", _schema, _params);
}

bool f_drawsetclippath(CObjRef drw_wnd, CStrRef clip_path) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(clip_path), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetclippath", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawsetcliprule(CObjRef drw_wnd, int fill_rule) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(fill_rule), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetcliprule", _schema, _params);
}

void f_drawsetclipunits(CObjRef drw_wnd, int clip_path_units) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(clip_path_units), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetclipunits", _schema, _params);
}

void f_drawsetfillalpha(CObjRef drw_wnd, double fill_opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(fill_opacity), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfillalpha", _schema, _params);
}

void f_drawsetfillcolor(CObjRef drw_wnd, CObjRef fill_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(fill_pxl_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfillcolor", _schema, _params);
}

void f_drawsetfillopacity(CObjRef drw_wnd, double fill_opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(fill_opacity), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfillopacity", _schema, _params);
}

bool f_drawsetfillpatternurl(CObjRef drw_wnd, CStrRef fill_url) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(fill_url), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetfillpatternurl", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawsetfillrule(CObjRef drw_wnd, int fill_rule) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(fill_rule), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfillrule", _schema, _params);
}

bool f_drawsetfont(CObjRef drw_wnd, CStrRef font_file) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(font_file), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetfont", _schema, _params);
  return (Variant)_ret[0];
}

bool f_drawsetfontfamily(CObjRef drw_wnd, CStrRef font_family) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(font_family), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetfontfamily", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawsetfontsize(CObjRef drw_wnd, double pointsize) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(pointsize), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfontsize", _schema, _params);
}

void f_drawsetfontstretch(CObjRef drw_wnd, int stretch_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stretch_type), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfontstretch", _schema, _params);
}

void f_drawsetfontstyle(CObjRef drw_wnd, int style_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(style_type), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfontstyle", _schema, _params);
}

void f_drawsetfontweight(CObjRef drw_wnd, double font_weight) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(font_weight), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetfontweight", _schema, _params);
}

void f_drawsetgravity(CObjRef drw_wnd, int gravity_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(gravity_type), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetgravity", _schema, _params);
}

void f_drawsetstrokealpha(CObjRef drw_wnd, double stroke_opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stroke_opacity), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokealpha", _schema, _params);
}

void f_drawsetstrokeantialias(CObjRef drw_wnd, bool stroke_antialias /* = true */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stroke_antialias), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokeantialias", _schema, _params);
}

void f_drawsetstrokecolor(CObjRef drw_wnd, CObjRef strokecolor_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(strokecolor_pxl_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokecolor", _schema, _params);
}

void f_drawsetstrokedasharray(CObjRef drw_wnd, CArrRef dash_array /* = null_array */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(dash_array), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokedasharray", _schema, _params);
}

void f_drawsetstrokedashoffset(CObjRef drw_wnd, double dash_offset) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(dash_offset), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokedashoffset", _schema, _params);
}

void f_drawsetstrokelinecap(CObjRef drw_wnd, int line_cap) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(line_cap), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokelinecap", _schema, _params);
}

void f_drawsetstrokelinejoin(CObjRef drw_wnd, int line_join) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(line_join), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokelinejoin", _schema, _params);
}

void f_drawsetstrokemiterlimit(CObjRef drw_wnd, double miterlimit) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(miterlimit), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokemiterlimit", _schema, _params);
}

void f_drawsetstrokeopacity(CObjRef drw_wnd, double stroke_opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stroke_opacity), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokeopacity", _schema, _params);
}

bool f_drawsetstrokepatternurl(CObjRef drw_wnd, CStrRef stroke_url) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stroke_url), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetstrokepatternurl", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawsetstrokewidth(CObjRef drw_wnd, double stroke_width) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(stroke_width), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetstrokewidth", _schema, _params);
}

void f_drawsettextalignment(CObjRef drw_wnd, int align_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(align_type), (ArrayElement*)NULL);
  Crutch::Invoke("drawsettextalignment", _schema, _params);
}

void f_drawsettextantialias(CObjRef drw_wnd, bool text_antialias /* = true */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(text_antialias), (ArrayElement*)NULL);
  Crutch::Invoke("drawsettextantialias", _schema, _params);
}

void f_drawsettextdecoration(CObjRef drw_wnd, int decoration_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(decoration_type), (ArrayElement*)NULL);
  Crutch::Invoke("drawsettextdecoration", _schema, _params);
}

void f_drawsettextencoding(CObjRef drw_wnd, CStrRef encoding) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(encoding), (ArrayElement*)NULL);
  Crutch::Invoke("drawsettextencoding", _schema, _params);
}

void f_drawsettextundercolor(CObjRef drw_wnd, CObjRef undercolor_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(undercolor_pxl_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawsettextundercolor", _schema, _params);
}

bool f_drawsetvectorgraphics(CObjRef drw_wnd, CStrRef vector_graphics) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(vector_graphics), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("drawsetvectorgraphics", _schema, _params);
  return (Variant)_ret[0];
}

void f_drawsetviewbox(CObjRef drw_wnd, double x1, double y1, double x2, double y2) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x1), NEW(ArrayElement)(y1), NEW(ArrayElement)(x2), NEW(ArrayElement)(y2), (ArrayElement*)NULL);
  Crutch::Invoke("drawsetviewbox", _schema, _params);
}

void f_drawskewx(CObjRef drw_wnd, double degrees) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(degrees), (ArrayElement*)NULL);
  Crutch::Invoke("drawskewx", _schema, _params);
}

void f_drawskewy(CObjRef drw_wnd, double degrees) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(degrees), (ArrayElement*)NULL);
  Crutch::Invoke("drawskewy", _schema, _params);
}

void f_drawtranslate(CObjRef drw_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Crutch::Invoke("drawtranslate", _schema, _params);
}

void f_pushdrawingwand(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("pushdrawingwand", _schema, _params);
}

void f_drawpushclippath(CObjRef drw_wnd, CStrRef clip_path_id) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(clip_path_id), (ArrayElement*)NULL);
  Crutch::Invoke("drawpushclippath", _schema, _params);
}

void f_drawpushdefs(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpushdefs", _schema, _params);
}

void f_drawpushpattern(CObjRef drw_wnd, CStrRef pattern_id, double x, double y, double width, double height) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(pattern_id), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(width), NEW(ArrayElement)(height), (ArrayElement*)NULL);
  Crutch::Invoke("drawpushpattern", _schema, _params);
}

void f_popdrawingwand(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("popdrawingwand", _schema, _params);
}

void f_drawpopclippath(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpopclippath", _schema, _params);
}

void f_drawpopdefs(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpopdefs", _schema, _params);
}

void f_drawpoppattern(CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("drawpoppattern", _schema, _params);
}

bool f_magickadaptivethresholdimage(CObjRef mgck_wnd, double width, double height, double offset) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(offset), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickadaptivethresholdimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickaddimage(CObjRef mgck_wnd, CObjRef add_wand) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(add_wand)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickaddimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickaddnoiseimage(CObjRef mgck_wnd, int noise_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(noise_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickaddnoiseimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickaffinetransformimage(CObjRef mgck_wnd, CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickaffinetransformimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickannotateimage(CObjRef mgck_wnd, CObjRef drw_wnd, double x, double y, double angle, CStrRef text) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(angle), NEW(ArrayElement)(text), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickannotateimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickappendimages(CObjRef mgck_wnd, bool stack_vertical /* = false */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(stack_vertical), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickappendimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_magickaverageimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickaverageimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickblackthresholdimage(CObjRef mgck_wnd, CObjRef threshold_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(threshold_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickblackthresholdimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickblurimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickblurimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickborderimage(CObjRef mgck_wnd, CObjRef bordercolor, double width, double height) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(bordercolor)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickborderimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcharcoalimage(CObjRef mgck_wnd, double radius, double sigma) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcharcoalimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickchopimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickchopimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickclipimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickclipimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickclippathimage(CObjRef mgck_wnd, CStrRef pathname, bool inside) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(pathname), NEW(ArrayElement)(inside), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickclippathimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickcoalesceimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcoalesceimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickcolorfloodfillimage(CObjRef mgck_wnd, CObjRef fillcolor_pxl_wnd, double fuzz, CObjRef bordercolor_pxl_wnd, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), NEW(ArrayElement)(3, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(fillcolor_pxl_wnd)), NEW(ArrayElement)(fuzz), NEW(ArrayElement)(OpaqueObject::GetIndex(bordercolor_pxl_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcolorfloodfillimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcolorizeimage(CObjRef mgck_wnd, CObjRef colorize, CObjRef opacity_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), NEW(ArrayElement)(2, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(colorize)), NEW(ArrayElement)(OpaqueObject::GetIndex(opacity_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcolorizeimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickcombineimages(CObjRef mgck_wnd, int channel_type) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcombineimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickcommentimage(CObjRef mgck_wnd, CStrRef comment) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(comment), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcommentimage", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickcompareimages(CObjRef mgck_wnd, CObjRef reference_wnd, int metric_type, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(reference_wnd)), NEW(ArrayElement)(metric_type), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcompareimages", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcompositeimage(CObjRef mgck_wnd, CObjRef composite_wnd, int composite_operator, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(composite_wnd)), NEW(ArrayElement)(composite_operator), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcompositeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickconstituteimage(CObjRef mgck_wnd, double columns, double rows, CStrRef smap, int storage_type, CArrRef pixel_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), NEW(ArrayElement)(smap), NEW(ArrayElement)(storage_type), NEW(ArrayElement)(pixel_array), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickconstituteimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcontrastimage(CObjRef mgck_wnd, bool sharpen) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(sharpen), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcontrastimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickconvolveimage(CObjRef mgck_wnd, CArrRef kernel_array, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(kernel_array), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickconvolveimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcropimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcropimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickcyclecolormapimage(CObjRef mgck_wnd, int num_positions) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(num_positions), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickcyclecolormapimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickdeconstructimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickdeconstructimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_magickdescribeimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickdescribeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickdespeckleimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickdespeckleimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickdrawimage(CObjRef mgck_wnd, CObjRef drw_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickdrawimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickechoimageblob(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickechoimageblob", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickechoimagesblob(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickechoimagesblob", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickedgeimage(CObjRef mgck_wnd, double radius) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickedgeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickembossimage(CObjRef mgck_wnd, double radius, double sigma) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickembossimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickenhanceimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickenhanceimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickequalizeimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickequalizeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickevaluateimage(CObjRef mgck_wnd, int evaluate_op, double constant, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(evaluate_op), NEW(ArrayElement)(constant), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickevaluateimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickflattenimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickflattenimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickflipimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickflipimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickflopimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickflopimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickframeimage(CObjRef mgck_wnd, CObjRef matte_color, double width, double height, int inner_bevel, int outer_bevel) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(matte_color)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(inner_bevel), NEW(ArrayElement)(outer_bevel), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickframeimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickfximage(CObjRef mgck_wnd, CStrRef expression, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(expression), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickfximage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickgammaimage(CObjRef mgck_wnd, double gamma, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(gamma), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgammaimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickgaussianblurimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgaussianblurimage", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetcharheight(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetcharheight", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetcharwidth(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetcharwidth", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetexception(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetexception", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetexceptionstring(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetexceptionstring", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetexceptiontype(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetexceptiontype", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetfilename(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetfilename", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetformat(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetformat", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickgetimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_magickgetimagebackgroundcolor(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagebackgroundcolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_magickgetimageblob(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageblob", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimageblueprimary(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageblueprimary", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickgetimagebordercolor(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagebordercolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_magickgetimagechannelmean(CObjRef mgck_wnd, int channel_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagechannelmean", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickgetimagecolormapcolor(CObjRef mgck_wnd, double index) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(index), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecolormapcolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

double f_magickgetimagecolors(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecolors", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagecolorspace(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecolorspace", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagecompose(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecompose", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagecompression(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecompression", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagecompressionquality(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagecompressionquality", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagedelay(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagedelay", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagedepth(CObjRef mgck_wnd, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagedepth", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagedispose(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagedispose", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimageextrema(CObjRef mgck_wnd, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageextrema", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetimagefilename(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagefilename", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetimageformat(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageformat", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagegamma(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagegamma", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimagegreenprimary(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagegreenprimary", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimageheight(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageheight", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimagehistogram(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagehistogram", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimageindex(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageindex", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimageinterlacescheme(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageinterlacescheme", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimageiterations(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageiterations", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickgetimagemattecolor(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagemattecolor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_magickgetimagemimetype(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagemimetype", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimagepixels(CObjRef mgck_wnd, int x_offset, int y_offset, double columns, double rows, CStrRef smap, int storage_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_offset), NEW(ArrayElement)(y_offset), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), NEW(ArrayElement)(smap), NEW(ArrayElement)(storage_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagepixels", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetimageprofile(CObjRef mgck_wnd, CStrRef name) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(name), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageprofile", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimageredprimary(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageredprimary", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagerenderingintent(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagerenderingintent", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimageresolution(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageresolution", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagescene(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagescene", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetimagesignature(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagesignature", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagesize(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagesize", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagetype(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagetype", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimageunits(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimageunits", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetimagevirtualpixelmethod(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagevirtualpixelmethod", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetimagewhitepoint(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagewhitepoint", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetimagewidth(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagewidth", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetimagesblob(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetimagesblob", _schema, _params);
  return (Variant)_ret[0];
}

int f_magickgetinterlacescheme(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetinterlacescheme", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetmaxtextadvance(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetmaxtextadvance", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickgetmimetype(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetmimetype", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetnumberimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetnumberimages", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetsamplingfactors(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetsamplingfactors", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetsize(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetsize", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetstringheight(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetstringheight", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgetstringwidth(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetstringwidth", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgettextascent(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgettextascent", _schema, _params);
  return (Variant)_ret[0];
}

double f_magickgettextdescent(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgettextdescent", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickgetwandsize(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickgetwandsize", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickhasnextimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickhasnextimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickhaspreviousimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickhaspreviousimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickimplodeimage(CObjRef mgck_wnd, double amount) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(amount), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickimplodeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicklabelimage(CObjRef mgck_wnd, CStrRef label) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(label), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicklabelimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicklevelimage(CObjRef mgck_wnd, double black_point, double gamma, double white_point, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(black_point), NEW(ArrayElement)(gamma), NEW(ArrayElement)(white_point), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicklevelimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickmagnifyimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmagnifyimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickmapimage(CObjRef mgck_wnd, CObjRef map_wand, bool dither) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(map_wand)), NEW(ArrayElement)(dither), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmapimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickmattefloodfillimage(CObjRef mgck_wnd, double opacity, double fuzz, CObjRef bordercolor_pxl_wnd, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(3, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(opacity), NEW(ArrayElement)(fuzz), NEW(ArrayElement)(OpaqueObject::GetIndex(bordercolor_pxl_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmattefloodfillimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickmedianfilterimage(CObjRef mgck_wnd, double radius) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmedianfilterimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickminifyimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickminifyimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickmodulateimage(CObjRef mgck_wnd, double brightness, double saturation, double hue) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(brightness), NEW(ArrayElement)(saturation), NEW(ArrayElement)(hue), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmodulateimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickmontageimage(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef tile_geometry, CStrRef thumbnail_geometry, int montage_mode, CStrRef frame) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(tile_geometry), NEW(ArrayElement)(thumbnail_geometry), NEW(ArrayElement)(montage_mode), NEW(ArrayElement)(frame), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmontageimage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_magickmorphimages(CObjRef mgck_wnd, double number_frames) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(number_frames), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmorphimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_magickmosaicimages(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmosaicimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickmotionblurimage(CObjRef mgck_wnd, double radius, double sigma, double angle) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), NEW(ArrayElement)(angle), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickmotionblurimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicknegateimage(CObjRef mgck_wnd, bool only_the_gray /* = false */, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(only_the_gray), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicknegateimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicknewimage(CObjRef mgck_wnd, double width, double height, CStrRef imagemagick_col_str /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(imagemagick_col_str), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicknewimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicknextimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicknextimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicknormalizeimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicknormalizeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickoilpaintimage(CObjRef mgck_wnd, double radius) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickoilpaintimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickpaintopaqueimage(CObjRef mgck_wnd, CObjRef target_pxl_wnd, CObjRef fill_pxl_wnd, double fuzz /* = 0.0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), NEW(ArrayElement)(2, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(target_pxl_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(fill_pxl_wnd)), NEW(ArrayElement)(fuzz), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickpaintopaqueimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickpainttransparentimage(CObjRef mgck_wnd, CObjRef target, double opacity /* = k_MW_TransparentOpacity */, double fuzz /* = 0.0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(target)), NEW(ArrayElement)(opacity), NEW(ArrayElement)(fuzz), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickpainttransparentimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickpingimage(CObjRef mgck_wnd, CStrRef filename) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickpingimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickposterizeimage(CObjRef mgck_wnd, double levels, bool dither) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(levels), NEW(ArrayElement)(dither), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickposterizeimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magickpreviewimages(CObjRef mgck_wnd, int preview) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(preview), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickpreviewimages", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickpreviousimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickpreviousimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickprofileimage(CObjRef mgck_wnd, CStrRef name, CStrRef profile /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(name), NEW(ArrayElement)(profile), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickprofileimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickquantizeimage(CObjRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(number_colors), NEW(ArrayElement)(colorspace_type), NEW(ArrayElement)(treedepth), NEW(ArrayElement)(dither), NEW(ArrayElement)(measure_error), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickquantizeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickquantizeimages(CObjRef mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(number_colors), NEW(ArrayElement)(colorspace_type), NEW(ArrayElement)(treedepth), NEW(ArrayElement)(dither), NEW(ArrayElement)(measure_error), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickquantizeimages", _schema, _params);
  return (Variant)_ret[0];
}

Array f_magickqueryfontmetrics(CObjRef mgck_wnd, CObjRef drw_wnd, CStrRef txt, bool multiline /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(drw_wnd)), NEW(ArrayElement)(txt), NEW(ArrayElement)(multiline), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickqueryfontmetrics", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickradialblurimage(CObjRef mgck_wnd, double angle) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(angle), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickradialblurimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickraiseimage(CObjRef mgck_wnd, double width, double height, int x, int y, bool raise) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(x), NEW(ArrayElement)(y), NEW(ArrayElement)(raise), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickraiseimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickreadimage(CObjRef mgck_wnd, CStrRef filename) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickreadimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickreadimageblob(CObjRef mgck_wnd, CStrRef blob) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(blob), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickreadimageblob", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickreadimagefile(CObjRef mgck_wnd, CObjRef handle) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(handle)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickreadimagefile", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickreadimages(CObjRef mgck_wnd, CArrRef img_filenames_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(img_filenames_array), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickreadimages", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickreducenoiseimage(CObjRef mgck_wnd, double radius) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickreducenoiseimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickremoveimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickremoveimage", _schema, _params);
  return (Variant)_ret[0];
}

String f_magickremoveimageprofile(CObjRef mgck_wnd, CStrRef name) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(name), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickremoveimageprofile", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickremoveimageprofiles(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickremoveimageprofiles", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickresampleimage(CObjRef mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_resolution), NEW(ArrayElement)(y_resolution), NEW(ArrayElement)(filter_type), NEW(ArrayElement)(blur), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickresampleimage", _schema, _params);
  return (Variant)_ret[0];
}

void f_magickresetiterator(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("magickresetiterator", _schema, _params);
}

bool f_magickresizeimage(CObjRef mgck_wnd, double columns, double rows, int filter_type, double blur) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), NEW(ArrayElement)(filter_type), NEW(ArrayElement)(blur), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickresizeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickrollimage(CObjRef mgck_wnd, int x_offset, int y_offset) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_offset), NEW(ArrayElement)(y_offset), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickrollimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickrotateimage(CObjRef mgck_wnd, CObjRef background, double degrees) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(background)), NEW(ArrayElement)(degrees), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickrotateimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksampleimage(CObjRef mgck_wnd, double columns, double rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksampleimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickscaleimage(CObjRef mgck_wnd, double columns, double rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickscaleimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickseparateimagechannel(CObjRef mgck_wnd, int channel_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickseparateimagechannel", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetcompressionquality(CObjRef mgck_wnd, double quality) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(quality), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetcompressionquality", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetfilename(CObjRef mgck_wnd, CStrRef filename /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetfilename", _schema, _params);
  return (Variant)_ret[0];
}

void f_magicksetfirstiterator(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("magicksetfirstiterator", _schema, _params);
}

bool f_magicksetformat(CObjRef mgck_wnd, CStrRef format) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(format), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetformat", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimage(CObjRef mgck_wnd, CObjRef replace_wand) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(replace_wand)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagebackgroundcolor(CObjRef mgck_wnd, CObjRef background_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(background_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagebackgroundcolor", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagebias(CObjRef mgck_wnd, double bias) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(bias), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagebias", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageblueprimary(CObjRef mgck_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageblueprimary", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagebordercolor(CObjRef mgck_wnd, CObjRef border_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(border_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagebordercolor", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagecolormapcolor(CObjRef mgck_wnd, double index, CObjRef mapcolor_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(index), NEW(ArrayElement)(OpaqueObject::GetIndex(mapcolor_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagecolormapcolor", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagecolorspace(CObjRef mgck_wnd, int colorspace_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(colorspace_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagecolorspace", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagecompose(CObjRef mgck_wnd, int composite_operator) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(composite_operator), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagecompose", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagecompression(CObjRef mgck_wnd, int compression_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(compression_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagecompression", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagecompressionquality(CObjRef mgck_wnd, double quality) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(quality), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagecompressionquality", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagedelay(CObjRef mgck_wnd, double delay) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(delay), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagedelay", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagedepth(CObjRef mgck_wnd, int depth, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(depth), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagedepth", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagedispose(CObjRef mgck_wnd, int dispose_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(dispose_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagedispose", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagefilename(CObjRef mgck_wnd, CStrRef filename /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagefilename", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageformat(CObjRef mgck_wnd, CStrRef format) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(format), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageformat", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagegamma(CObjRef mgck_wnd, double gamma) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(gamma), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagegamma", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagegreenprimary(CObjRef mgck_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagegreenprimary", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageindex(CObjRef mgck_wnd, int index) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(index), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageindex", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageinterlacescheme(CObjRef mgck_wnd, int interlace_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(interlace_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageinterlacescheme", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageiterations(CObjRef mgck_wnd, double iterations) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(iterations), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageiterations", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagemattecolor(CObjRef mgck_wnd, CObjRef matte_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(matte_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagemattecolor", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageoption(CObjRef mgck_wnd, CStrRef format, CStrRef key, CStrRef value) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(format), NEW(ArrayElement)(key), NEW(ArrayElement)(value), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageoption", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagepixels(CObjRef mgck_wnd, int x_offset, int y_offset, double columns, double rows, CStrRef smap, int storage_type, CArrRef pixel_array) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_offset), NEW(ArrayElement)(y_offset), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), NEW(ArrayElement)(smap), NEW(ArrayElement)(storage_type), NEW(ArrayElement)(pixel_array), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagepixels", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageprofile(CObjRef mgck_wnd, CStrRef name, CStrRef profile) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(name), NEW(ArrayElement)(profile), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageprofile", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageredprimary(CObjRef mgck_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageredprimary", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagerenderingintent(CObjRef mgck_wnd, int rendering_intent) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(rendering_intent), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagerenderingintent", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageresolution(CObjRef mgck_wnd, double x_resolution, double y_resolution) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_resolution), NEW(ArrayElement)(y_resolution), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageresolution", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagescene(CObjRef mgck_wnd, double scene) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(scene), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagescene", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagetype(CObjRef mgck_wnd, int image_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(image_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagetype", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimageunits(CObjRef mgck_wnd, int resolution_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(resolution_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimageunits", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagevirtualpixelmethod(CObjRef mgck_wnd, int virtual_pixel_method) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(virtual_pixel_method), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagevirtualpixelmethod", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetimagewhitepoint(CObjRef mgck_wnd, double x, double y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetimagewhitepoint", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetinterlacescheme(CObjRef mgck_wnd, int interlace_type) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(interlace_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetinterlacescheme", _schema, _params);
  return (Variant)_ret[0];
}

void f_magicksetlastiterator(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Crutch::Invoke("magicksetlastiterator", _schema, _params);
}

bool f_magicksetpassphrase(CObjRef mgck_wnd, CStrRef passphrase) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(passphrase), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetpassphrase", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetresolution(CObjRef mgck_wnd, double x_resolution, double y_resolution) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(x_resolution), NEW(ArrayElement)(y_resolution), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetresolution", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetsamplingfactors(CObjRef mgck_wnd, double number_factors, CArrRef sampling_factors) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(number_factors), NEW(ArrayElement)(sampling_factors), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetsamplingfactors", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetsize(CObjRef mgck_wnd, int columns, int rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetsize", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksetwandsize(CObjRef mgck_wnd, int columns, int rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksetwandsize", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksharpenimage(CObjRef mgck_wnd, double radius, double sigma, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksharpenimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickshaveimage(CObjRef mgck_wnd, int columns, int rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(columns), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickshaveimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickshearimage(CObjRef mgck_wnd, CObjRef background, double x_shear, double y_shear) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(background)), NEW(ArrayElement)(x_shear), NEW(ArrayElement)(y_shear), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickshearimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicksolarizeimage(CObjRef mgck_wnd, double threshold) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(threshold), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksolarizeimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickspliceimage(CObjRef mgck_wnd, double width, double height, int x, int y) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(width), NEW(ArrayElement)(height), NEW(ArrayElement)(x), NEW(ArrayElement)(y), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickspliceimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickspreadimage(CObjRef mgck_wnd, double radius) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickspreadimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magicksteganoimage(CObjRef mgck_wnd, CObjRef watermark_wand, int offset) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(watermark_wand)), NEW(ArrayElement)(offset), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicksteganoimage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickstereoimage(CObjRef mgck_wnd, CObjRef offset_wand) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(offset_wand)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickstereoimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickstripimage(CObjRef mgck_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickstripimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickswirlimage(CObjRef mgck_wnd, double degrees) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(degrees), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickswirlimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magicktextureimage(CObjRef mgck_wnd, CObjRef texture_wand) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(texture_wand)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicktextureimage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magickthresholdimage(CObjRef mgck_wnd, double threshold, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(threshold), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickthresholdimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magicktintimage(CObjRef mgck_wnd, int tint_pxl_wnd, CObjRef opacity_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(tint_pxl_wnd), NEW(ArrayElement)(OpaqueObject::GetIndex(opacity_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicktintimage", _schema, _params);
  return (Variant)_ret[0];
}

Object f_magicktransformimage(CObjRef mgck_wnd, CStrRef crop, CStrRef geometry) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(crop), NEW(ArrayElement)(geometry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicktransformimage", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_magicktrimimage(CObjRef mgck_wnd, double fuzz) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(fuzz), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magicktrimimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickunsharpmaskimage(CObjRef mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(radius), NEW(ArrayElement)(sigma), NEW(ArrayElement)(amount), NEW(ArrayElement)(threshold), NEW(ArrayElement)(channel_type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickunsharpmaskimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwaveimage(CObjRef mgck_wnd, double amplitude, double wave_length) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(amplitude), NEW(ArrayElement)(wave_length), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwaveimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwhitethresholdimage(CObjRef mgck_wnd, CObjRef threshold_pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(threshold_pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwhitethresholdimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwriteimage(CObjRef mgck_wnd, CStrRef filename) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwriteimage", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwriteimagefile(CObjRef mgck_wnd, CObjRef handle) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(handle)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwriteimagefile", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwriteimages(CObjRef mgck_wnd, CStrRef filename /* = "" */, bool join_images /* = false */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(filename), NEW(ArrayElement)(join_images), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwriteimages", _schema, _params);
  return (Variant)_ret[0];
}

bool f_magickwriteimagesfile(CObjRef mgck_wnd, CObjRef handle) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mgck_wnd)), NEW(ArrayElement)(OpaqueObject::GetIndex(handle)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("magickwriteimagesfile", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetalpha(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetalpha", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetalphaquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetalphaquantum", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetblack(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetblack", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetblackquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetblackquantum", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetblue(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetblue", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetbluequantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetbluequantum", _schema, _params);
  return (Variant)_ret[0];
}

String f_pixelgetcolorasstring(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetcolorasstring", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetcolorcount(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetcolorcount", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetcyan(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetcyan", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetcyanquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetcyanquantum", _schema, _params);
  return (Variant)_ret[0];
}

Array f_pixelgetexception(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetexception", _schema, _params);
  return (Variant)_ret[0];
}

String f_pixelgetexceptionstring(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetexceptionstring", _schema, _params);
  return (Variant)_ret[0];
}

int f_pixelgetexceptiontype(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetexceptiontype", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetgreen(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetgreen", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetgreenquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetgreenquantum", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetindex(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetindex", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetmagenta(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetmagenta", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetmagentaquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetmagentaquantum", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetopacity(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetopacity", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetopacityquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetopacityquantum", _schema, _params);
  return (Variant)_ret[0];
}

Array f_pixelgetquantumcolor(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetquantumcolor", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetred(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetred", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetredquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetredquantum", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetyellow(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetyellow", _schema, _params);
  return (Variant)_ret[0];
}

double f_pixelgetyellowquantum(CObjRef pxl_wnd) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetyellowquantum", _schema, _params);
  return (Variant)_ret[0];
}

void f_pixelsetalpha(CObjRef pxl_wnd, double alpha) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(alpha), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetalpha", _schema, _params);
}

void f_pixelsetalphaquantum(CObjRef pxl_wnd, double alpha) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(alpha), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetalphaquantum", _schema, _params);
}

void f_pixelsetblack(CObjRef pxl_wnd, double black) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(black), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetblack", _schema, _params);
}

void f_pixelsetblackquantum(CObjRef pxl_wnd, double black) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(black), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetblackquantum", _schema, _params);
}

void f_pixelsetblue(CObjRef pxl_wnd, double blue) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(blue), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetblue", _schema, _params);
}

void f_pixelsetbluequantum(CObjRef pxl_wnd, double blue) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(blue), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetbluequantum", _schema, _params);
}

void f_pixelsetcolor(CObjRef pxl_wnd, CStrRef imagemagick_col_str) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(imagemagick_col_str), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetcolor", _schema, _params);
}

void f_pixelsetcolorcount(CObjRef pxl_wnd, int count) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(count), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetcolorcount", _schema, _params);
}

void f_pixelsetcyan(CObjRef pxl_wnd, double cyan) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(cyan), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetcyan", _schema, _params);
}

void f_pixelsetcyanquantum(CObjRef pxl_wnd, double cyan) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(cyan), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetcyanquantum", _schema, _params);
}

void f_pixelsetgreen(CObjRef pxl_wnd, double green) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(green), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetgreen", _schema, _params);
}

void f_pixelsetgreenquantum(CObjRef pxl_wnd, double green) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(green), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetgreenquantum", _schema, _params);
}

void f_pixelsetindex(CObjRef pxl_wnd, double index) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(index), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetindex", _schema, _params);
}

void f_pixelsetmagenta(CObjRef pxl_wnd, double magenta) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(magenta), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetmagenta", _schema, _params);
}

void f_pixelsetmagentaquantum(CObjRef pxl_wnd, double magenta) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(magenta), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetmagentaquantum", _schema, _params);
}

void f_pixelsetopacity(CObjRef pxl_wnd, double opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(opacity), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetopacity", _schema, _params);
}

void f_pixelsetopacityquantum(CObjRef pxl_wnd, double opacity) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(opacity), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetopacityquantum", _schema, _params);
}

void f_pixelsetquantumcolor(CObjRef pxl_wnd, double red, double green, double blue, double opacity /* = 0.0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(red), NEW(ArrayElement)(green), NEW(ArrayElement)(blue), NEW(ArrayElement)(opacity), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetquantumcolor", _schema, _params);
}

void f_pixelsetred(CObjRef pxl_wnd, double red) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(red), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetred", _schema, _params);
}

void f_pixelsetredquantum(CObjRef pxl_wnd, double red) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(red), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetredquantum", _schema, _params);
}

void f_pixelsetyellow(CObjRef pxl_wnd, double yellow) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(yellow), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetyellow", _schema, _params);
}

void f_pixelsetyellowquantum(CObjRef pxl_wnd, double yellow) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_wnd)), NEW(ArrayElement)(yellow), (ArrayElement*)NULL);
  Crutch::Invoke("pixelsetyellowquantum", _schema, _params);
}

Array f_pixelgetiteratorexception(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetiteratorexception", _schema, _params);
  return (Variant)_ret[0];
}

String f_pixelgetiteratorexceptionstring(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetiteratorexceptionstring", _schema, _params);
  return (Variant)_ret[0];
}

int f_pixelgetiteratorexceptiontype(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetiteratorexceptiontype", _schema, _params);
  return (Variant)_ret[0];
}

Array f_pixelgetnextiteratorrow(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelgetnextiteratorrow", _schema, _params);
  return (Variant)_ret[0];
}

void f_pixelresetiterator(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Crutch::Invoke("pixelresetiterator", _schema, _params);
}

bool f_pixelsetiteratorrow(CObjRef pxl_iter, int row) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), NEW(ArrayElement)(row), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelsetiteratorrow", _schema, _params);
  return (Variant)_ret[0];
}

bool f_pixelsynciterator(CObjRef pxl_iter) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(pxl_iter)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("pixelsynciterator", _schema, _params);
  return (Variant)_ret[0];
}


///////////////////////////////////////////////////////////////////////////////
}
