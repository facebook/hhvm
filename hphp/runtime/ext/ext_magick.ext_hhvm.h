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

/*
HPHP::String HPHP::f_magickgetcopyright()
_ZN4HPHP20f_magickgetcopyrightEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetcopyright(Value* _rv) asm("_ZN4HPHP20f_magickgetcopyrightEv");

/*
HPHP::String HPHP::f_magickgethomeurl()
_ZN4HPHP18f_magickgethomeurlEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgethomeurl(Value* _rv) asm("_ZN4HPHP18f_magickgethomeurlEv");

/*
HPHP::String HPHP::f_magickgetpackagename()
_ZN4HPHP22f_magickgetpackagenameEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetpackagename(Value* _rv) asm("_ZN4HPHP22f_magickgetpackagenameEv");

/*
double HPHP::f_magickgetquantumdepth()
_ZN4HPHP23f_magickgetquantumdepthEv

(return value) => xmm0
*/

double fh_magickgetquantumdepth() asm("_ZN4HPHP23f_magickgetquantumdepthEv");

/*
HPHP::String HPHP::f_magickgetreleasedate()
_ZN4HPHP22f_magickgetreleasedateEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetreleasedate(Value* _rv) asm("_ZN4HPHP22f_magickgetreleasedateEv");

/*
double HPHP::f_magickgetresourcelimit(int)
_ZN4HPHP24f_magickgetresourcelimitEi

(return value) => xmm0
resource_type => rdi
*/

double fh_magickgetresourcelimit(int resource_type) asm("_ZN4HPHP24f_magickgetresourcelimitEi");

/*
HPHP::Array HPHP::f_magickgetversion()
_ZN4HPHP18f_magickgetversionEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetversion(Value* _rv) asm("_ZN4HPHP18f_magickgetversionEv");

/*
long long HPHP::f_magickgetversionnumber()
_ZN4HPHP24f_magickgetversionnumberEv

(return value) => rax
*/

long long fh_magickgetversionnumber() asm("_ZN4HPHP24f_magickgetversionnumberEv");

/*
HPHP::String HPHP::f_magickgetversionstring()
_ZN4HPHP24f_magickgetversionstringEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetversionstring(Value* _rv) asm("_ZN4HPHP24f_magickgetversionstringEv");

/*
HPHP::String HPHP::f_magickqueryconfigureoption(HPHP::String const&)
_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE

(return value) => rax
_rv => rdi
option => rsi
*/

Value* fh_magickqueryconfigureoption(Value* _rv, Value* option) asm("_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE");

/*
HPHP::Array HPHP::f_magickqueryconfigureoptions(HPHP::String const&)
_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryconfigureoptions(Value* _rv, Value* pattern) asm("_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE");

/*
HPHP::Array HPHP::f_magickqueryfonts(HPHP::String const&)
_ZN4HPHP18f_magickqueryfontsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryfonts(Value* _rv, Value* pattern) asm("_ZN4HPHP18f_magickqueryfontsERKNS_6StringE");

/*
HPHP::Array HPHP::f_magickqueryformats(HPHP::String const&)
_ZN4HPHP20f_magickqueryformatsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryformats(Value* _rv, Value* pattern) asm("_ZN4HPHP20f_magickqueryformatsERKNS_6StringE");

/*
bool HPHP::f_magicksetresourcelimit(int, double)
_ZN4HPHP24f_magicksetresourcelimitEid

(return value) => rax
resource_type => rdi
limit => xmm0
*/

bool fh_magicksetresourcelimit(int resource_type, double limit) asm("_ZN4HPHP24f_magicksetresourcelimitEid");

/*
HPHP::Object HPHP::f_newdrawingwand()
_ZN4HPHP16f_newdrawingwandEv

(return value) => rax
_rv => rdi
*/

Value* fh_newdrawingwand(Value* _rv) asm("_ZN4HPHP16f_newdrawingwandEv");

/*
HPHP::Object HPHP::f_newmagickwand()
_ZN4HPHP15f_newmagickwandEv

(return value) => rax
_rv => rdi
*/

Value* fh_newmagickwand(Value* _rv) asm("_ZN4HPHP15f_newmagickwandEv");

/*
HPHP::Object HPHP::f_newpixeliterator(HPHP::Object const&)
_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_newpixeliterator(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_newpixelregioniterator(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
mgck_wnd => rsi
x => rdx
y => rcx
columns => r8
rows => r9
*/

Value* fh_newpixelregioniterator(Value* _rv, Value* mgck_wnd, int x, int y, int columns, int rows) asm("_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii");

/*
HPHP::Object HPHP::f_newpixelwand(HPHP::String const&)
_ZN4HPHP14f_newpixelwandERKNS_6StringE

(return value) => rax
_rv => rdi
imagemagick_col_str => rsi
*/

Value* fh_newpixelwand(Value* _rv, Value* imagemagick_col_str) asm("_ZN4HPHP14f_newpixelwandERKNS_6StringE");

/*
HPHP::Array HPHP::f_newpixelwandarray(int)
_ZN4HPHP19f_newpixelwandarrayEi

(return value) => rax
_rv => rdi
num_pxl_wnds => rsi
*/

Value* fh_newpixelwandarray(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP19f_newpixelwandarrayEi");

/*
HPHP::Array HPHP::f_newpixelwands(int)
_ZN4HPHP15f_newpixelwandsEi

(return value) => rax
_rv => rdi
num_pxl_wnds => rsi
*/

Value* fh_newpixelwands(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP15f_newpixelwandsEi");

/*
void HPHP::f_destroydrawingwand(HPHP::Object const&)
_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_destroydrawingwand(Value* drw_wnd) asm("_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE");

/*
void HPHP::f_destroymagickwand(HPHP::Object const&)
_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_destroymagickwand(Value* mgck_wnd) asm("_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE");

/*
void HPHP::f_destroypixeliterator(HPHP::Object const&)
_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_destroypixeliterator(Value* pxl_iter) asm("_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE");

/*
void HPHP::f_destroypixelwand(HPHP::Object const&)
_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE

pxl_wnd => rdi
*/

void fh_destroypixelwand(Value* pxl_wnd) asm("_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE");

/*
void HPHP::f_destroypixelwandarray(HPHP::Array const&)
_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE

pxl_wnd_array => rdi
*/

void fh_destroypixelwandarray(Value* pxl_wnd_array) asm("_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE");

/*
void HPHP::f_destroypixelwands(HPHP::Array const&)
_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE

pxl_wnd_array => rdi
*/

void fh_destroypixelwands(Value* pxl_wnd_array) asm("_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE");

/*
bool HPHP::f_isdrawingwand(HPHP::Variant const&)
_ZN4HPHP15f_isdrawingwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_isdrawingwand(TypedValue* var) asm("_ZN4HPHP15f_isdrawingwandERKNS_7VariantE");

/*
bool HPHP::f_ismagickwand(HPHP::Variant const&)
_ZN4HPHP14f_ismagickwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ismagickwand(TypedValue* var) asm("_ZN4HPHP14f_ismagickwandERKNS_7VariantE");

/*
bool HPHP::f_ispixeliterator(HPHP::Variant const&)
_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ispixeliterator(TypedValue* var) asm("_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE");

/*
bool HPHP::f_ispixelwand(HPHP::Variant const&)
_ZN4HPHP13f_ispixelwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ispixelwand(TypedValue* var) asm("_ZN4HPHP13f_ispixelwandERKNS_7VariantE");

/*
void HPHP::f_cleardrawingwand(HPHP::Object const&)
_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_cleardrawingwand(Value* drw_wnd) asm("_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE");

/*
void HPHP::f_clearmagickwand(HPHP::Object const&)
_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_clearmagickwand(Value* mgck_wnd) asm("_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE");

/*
void HPHP::f_clearpixeliterator(HPHP::Object const&)
_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_clearpixeliterator(Value* pxl_iter) asm("_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE");

/*
void HPHP::f_clearpixelwand(HPHP::Object const&)
_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE

pxl_wnd => rdi
*/

void fh_clearpixelwand(Value* pxl_wnd) asm("_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_clonedrawingwand(HPHP::Object const&)
_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_clonedrawingwand(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_clonemagickwand(HPHP::Object const&)
_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_clonemagickwand(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_wandgetexception(HPHP::Object const&)
_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
wnd => rsi
*/

Value* fh_wandgetexception(Value* _rv, Value* wnd) asm("_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE");

/*
HPHP::String HPHP::f_wandgetexceptionstring(HPHP::Object const&)
_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
wnd => rsi
*/

Value* fh_wandgetexceptionstring(Value* _rv, Value* wnd) asm("_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE");

/*
long long HPHP::f_wandgetexceptiontype(HPHP::Object const&)
_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE

(return value) => rax
wnd => rdi
*/

long long fh_wandgetexceptiontype(Value* wnd) asm("_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE");

/*
bool HPHP::f_wandhasexception(HPHP::Object const&)
_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE

(return value) => rax
wnd => rdi
*/

bool fh_wandhasexception(Value* wnd) asm("_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE");

/*
void HPHP::f_drawaffine(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
rx => xmm2
ry => xmm3
tx => xmm4
ty => xmm5
*/

void fh_drawaffine(Value* drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) asm("_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd");

/*
void HPHP::f_drawannotation(HPHP::Object const&, double, double, HPHP::String const&)
_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE

drw_wnd => rdi
x => xmm0
y => xmm1
text => rsi
*/

void fh_drawannotation(Value* drw_wnd, double x, double y, Value* text) asm("_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE");

/*
void HPHP::f_drawarc(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
ex => xmm2
ey => xmm3
sd => xmm4
ed => xmm5
*/

void fh_drawarc(Value* drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) asm("_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd");

/*
void HPHP::f_drawbezier(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawbezier(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE");

/*
void HPHP::f_drawcircle(HPHP::Object const&, double, double, double, double)
_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd

drw_wnd => rdi
ox => xmm0
oy => xmm1
px => xmm2
py => xmm3
*/

void fh_drawcircle(Value* drw_wnd, double ox, double oy, double px, double py) asm("_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd");

/*
void HPHP::f_drawcolor(HPHP::Object const&, double, double, int)
_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi

drw_wnd => rdi
x => xmm0
y => xmm1
paint_method => rsi
*/

void fh_drawcolor(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi");

/*
void HPHP::f_drawcomment(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
comment => rsi
*/

void fh_drawcomment(Value* drw_wnd, Value* comment) asm("_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_drawcomposite(HPHP::Object const&, int, double, double, double, double, HPHP::Object const&)
_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_

(return value) => rax
drw_wnd => rdi
composite_operator => rsi
x => xmm0
y => xmm1
width => xmm2
height => xmm3
mgck_wnd => rdx
*/

bool fh_drawcomposite(Value* drw_wnd, int composite_operator, double x, double y, double width, double height, Value* mgck_wnd) asm("_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_");

/*
void HPHP::f_drawellipse(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd

drw_wnd => rdi
ox => xmm0
oy => xmm1
rx => xmm2
ry => xmm3
start => xmm4
end => xmm5
*/

void fh_drawellipse(Value* drw_wnd, double ox, double oy, double rx, double ry, double start, double end) asm("_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd");

/*
HPHP::String HPHP::f_drawgetclippath(HPHP::Object const&)
_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetclippath(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE");

/*
long long HPHP::f_drawgetcliprule(HPHP::Object const&)
_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetcliprule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE");

/*
long long HPHP::f_drawgetclipunits(HPHP::Object const&)
_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetclipunits(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_drawgetexception(HPHP::Object const&)
_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetexception(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE");

/*
HPHP::String HPHP::f_drawgetexceptionstring(HPHP::Object const&)
_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetexceptionstring(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE");

/*
long long HPHP::f_drawgetexceptiontype(HPHP::Object const&)
_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetexceptiontype(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE");

/*
double HPHP::f_drawgetfillalpha(HPHP::Object const&)
_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfillalpha(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_drawgetfillcolor(HPHP::Object const&)
_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfillcolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE");

/*
double HPHP::f_drawgetfillopacity(HPHP::Object const&)
_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfillopacity(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE");

/*
long long HPHP::f_drawgetfillrule(HPHP::Object const&)
_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetfillrule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE");

/*
HPHP::String HPHP::f_drawgetfont(HPHP::Object const&)
_ZN4HPHP13f_drawgetfontERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfont(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP13f_drawgetfontERKNS_6ObjectE");

/*
HPHP::String HPHP::f_drawgetfontfamily(HPHP::Object const&)
_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfontfamily(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE");

/*
double HPHP::f_drawgetfontsize(HPHP::Object const&)
_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfontsize(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE");

/*
long long HPHP::f_drawgetfontstretch(HPHP::Object const&)
_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetfontstretch(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE");

/*
long long HPHP::f_drawgetfontstyle(HPHP::Object const&)
_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetfontstyle(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE");

/*
double HPHP::f_drawgetfontweight(HPHP::Object const&)
_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfontweight(Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE");

/*
long long HPHP::f_drawgetgravity(HPHP::Object const&)
_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetgravity(Value* drw_wnd) asm("_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE");

/*
double HPHP::f_drawgetstrokealpha(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokealpha(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE");

/*
bool HPHP::f_drawgetstrokeantialias(HPHP::Object const&)
_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawgetstrokeantialias(Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_drawgetstrokecolor(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetstrokecolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_drawgetstrokedasharray(HPHP::Object const&)
_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetstrokedasharray(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE");

/*
double HPHP::f_drawgetstrokedashoffset(HPHP::Object const&)
_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokedashoffset(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE");

/*
long long HPHP::f_drawgetstrokelinecap(HPHP::Object const&)
_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetstrokelinecap(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE");

/*
long long HPHP::f_drawgetstrokelinejoin(HPHP::Object const&)
_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgetstrokelinejoin(Value* drw_wnd) asm("_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE");

/*
double HPHP::f_drawgetstrokemiterlimit(HPHP::Object const&)
_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokemiterlimit(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE");

/*
double HPHP::f_drawgetstrokeopacity(HPHP::Object const&)
_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokeopacity(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE");

/*
double HPHP::f_drawgetstrokewidth(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokewidth(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE");

/*
long long HPHP::f_drawgettextalignment(HPHP::Object const&)
_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgettextalignment(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE");

/*
bool HPHP::f_drawgettextantialias(HPHP::Object const&)
_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawgettextantialias(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE");

/*
long long HPHP::f_drawgettextdecoration(HPHP::Object const&)
_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long long fh_drawgettextdecoration(Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE");

/*
HPHP::String HPHP::f_drawgettextencoding(HPHP::Object const&)
_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgettextencoding(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_drawgettextundercolor(HPHP::Object const&)
_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgettextundercolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE");

/*
HPHP::String HPHP::f_drawgetvectorgraphics(HPHP::Object const&)
_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetvectorgraphics(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE");

/*
void HPHP::f_drawline(HPHP::Object const&, double, double, double, double)
_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
ex => xmm2
ey => xmm3
*/

void fh_drawline(Value* drw_wnd, double sx, double sy, double ex, double ey) asm("_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd");

/*
void HPHP::f_drawmatte(HPHP::Object const&, double, double, int)
_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi

drw_wnd => rdi
x => xmm0
y => xmm1
paint_method => rsi
*/

void fh_drawmatte(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi");

/*
void HPHP::f_drawpathclose(HPHP::Object const&)
_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathclose(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE");

/*
void HPHP::f_drawpathcurvetoabsolute(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
x => xmm4
y => xmm5
*/

void fh_drawpathcurvetoabsolute(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd");

/*
void HPHP::f_drawpathcurvetoquadraticbezierabsolute(HPHP::Object const&, double, double, double, double)
_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetoquadraticbezierabsolute(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd");

/*
void HPHP::f_drawpathcurvetoquadraticbezierrelative(HPHP::Object const&, double, double, double, double)
_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetoquadraticbezierrelative(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd");

/*
void HPHP::f_drawpathcurvetoquadraticbeziersmoothabsolute(HPHP::Object const&, double, double)
_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathcurvetoquadraticbeziersmoothabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathcurvetoquadraticbeziersmoothrelative(HPHP::Object const&, double, double)
_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathcurvetoquadraticbeziersmoothrelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathcurvetorelative(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
x => xmm4
y => xmm5
*/

void fh_drawpathcurvetorelative(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd");

/*
void HPHP::f_drawpathcurvetosmoothabsolute(HPHP::Object const&, double, double, double, double)
_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd

drw_wnd => rdi
x2 => xmm0
y2 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetosmoothabsolute(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd");

/*
void HPHP::f_drawpathcurvetosmoothrelative(HPHP::Object const&, double, double, double, double)
_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd

drw_wnd => rdi
x2 => xmm0
y2 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetosmoothrelative(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd");

/*
void HPHP::f_drawpathellipticarcabsolute(HPHP::Object const&, double, double, double, bool, bool, double, double)
_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd

drw_wnd => rdi
rx => xmm0
ry => xmm1
x_axis_rotation => xmm2
large_arc_flag => rsi
sweep_flag => rdx
x => xmm3
y => xmm4
*/

void fh_drawpathellipticarcabsolute(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd");

/*
void HPHP::f_drawpathellipticarcrelative(HPHP::Object const&, double, double, double, bool, bool, double, double)
_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd

drw_wnd => rdi
rx => xmm0
ry => xmm1
x_axis_rotation => xmm2
large_arc_flag => rsi
sweep_flag => rdx
x => xmm3
y => xmm4
*/

void fh_drawpathellipticarcrelative(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd");

/*
void HPHP::f_drawpathfinish(HPHP::Object const&)
_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathfinish(Value* drw_wnd) asm("_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE");

/*
void HPHP::f_drawpathlinetoabsolute(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathlinetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathlinetohorizontalabsolute(HPHP::Object const&, double)
_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd

drw_wnd => rdi
x => xmm0
*/

void fh_drawpathlinetohorizontalabsolute(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd");

/*
void HPHP::f_drawpathlinetohorizontalrelative(HPHP::Object const&, double)
_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd

drw_wnd => rdi
x => xmm0
*/

void fh_drawpathlinetohorizontalrelative(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd");

/*
void HPHP::f_drawpathlinetorelative(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathlinetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathlinetoverticalabsolute(HPHP::Object const&, double)
_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd

drw_wnd => rdi
y => xmm0
*/

void fh_drawpathlinetoverticalabsolute(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd");

/*
void HPHP::f_drawpathlinetoverticalrelative(HPHP::Object const&, double)
_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd

drw_wnd => rdi
y => xmm0
*/

void fh_drawpathlinetoverticalrelative(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd");

/*
void HPHP::f_drawpathmovetoabsolute(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathmovetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathmovetorelative(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathmovetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd");

/*
void HPHP::f_drawpathstart(HPHP::Object const&)
_ZN4HPHP15f_drawpathstartERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathstart(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathstartERKNS_6ObjectE");

/*
void HPHP::f_drawpoint(HPHP::Object const&, double, double)
_ZN4HPHP11f_drawpointERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpoint(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawpointERKNS_6ObjectEdd");

/*
void HPHP::f_drawpolygon(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawpolygon(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE");

/*
void HPHP::f_drawpolyline(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawpolyline(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE");

/*
void HPHP::f_drawrectangle(HPHP::Object const&, double, double, double, double)
_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
*/

void fh_drawrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd");

/*
bool HPHP::f_drawrender(HPHP::Object const&)
_ZN4HPHP12f_drawrenderERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawrender(Value* drw_wnd) asm("_ZN4HPHP12f_drawrenderERKNS_6ObjectE");

/*
void HPHP::f_drawrotate(HPHP::Object const&, double)
_ZN4HPHP12f_drawrotateERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawrotate(Value* drw_wnd, double degrees) asm("_ZN4HPHP12f_drawrotateERKNS_6ObjectEd");

/*
void HPHP::f_drawroundrectangle(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
rx => xmm4
ry => xmm5
*/

void fh_drawroundrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) asm("_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd");

/*
void HPHP::f_drawscale(HPHP::Object const&, double, double)
_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawscale(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd");

/*
bool HPHP::f_drawsetclippath(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
clip_path => rsi
*/

bool fh_drawsetclippath(Value* drw_wnd, Value* clip_path) asm("_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsetcliprule(HPHP::Object const&, int)
_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi

drw_wnd => rdi
fill_rule => rsi
*/

void fh_drawsetcliprule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi");

/*
void HPHP::f_drawsetclipunits(HPHP::Object const&, int)
_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi

drw_wnd => rdi
clip_path_units => rsi
*/

void fh_drawsetclipunits(Value* drw_wnd, int clip_path_units) asm("_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi");

/*
void HPHP::f_drawsetfillalpha(HPHP::Object const&, double)
_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd

drw_wnd => rdi
fill_opacity => xmm0
*/

void fh_drawsetfillalpha(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd");

/*
void HPHP::f_drawsetfillcolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_

drw_wnd => rdi
fill_pxl_wnd => rsi
*/

void fh_drawsetfillcolor(Value* drw_wnd, Value* fill_pxl_wnd) asm("_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_");

/*
void HPHP::f_drawsetfillopacity(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd

drw_wnd => rdi
fill_opacity => xmm0
*/

void fh_drawsetfillopacity(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd");

/*
bool HPHP::f_drawsetfillpatternurl(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
fill_url => rsi
*/

bool fh_drawsetfillpatternurl(Value* drw_wnd, Value* fill_url) asm("_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsetfillrule(HPHP::Object const&, int)
_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi

drw_wnd => rdi
fill_rule => rsi
*/

void fh_drawsetfillrule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi");

/*
bool HPHP::f_drawsetfont(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
font_file => rsi
*/

bool fh_drawsetfont(Value* drw_wnd, Value* font_file) asm("_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_drawsetfontfamily(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
font_family => rsi
*/

bool fh_drawsetfontfamily(Value* drw_wnd, Value* font_family) asm("_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsetfontsize(HPHP::Object const&, double)
_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd

drw_wnd => rdi
pointsize => xmm0
*/

void fh_drawsetfontsize(Value* drw_wnd, double pointsize) asm("_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd");

/*
void HPHP::f_drawsetfontstretch(HPHP::Object const&, int)
_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi

drw_wnd => rdi
stretch_type => rsi
*/

void fh_drawsetfontstretch(Value* drw_wnd, int stretch_type) asm("_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi");

/*
void HPHP::f_drawsetfontstyle(HPHP::Object const&, int)
_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi

drw_wnd => rdi
style_type => rsi
*/

void fh_drawsetfontstyle(Value* drw_wnd, int style_type) asm("_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi");

/*
void HPHP::f_drawsetfontweight(HPHP::Object const&, double)
_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd

drw_wnd => rdi
font_weight => xmm0
*/

void fh_drawsetfontweight(Value* drw_wnd, double font_weight) asm("_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd");

/*
void HPHP::f_drawsetgravity(HPHP::Object const&, int)
_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi

drw_wnd => rdi
gravity_type => rsi
*/

void fh_drawsetgravity(Value* drw_wnd, int gravity_type) asm("_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi");

/*
void HPHP::f_drawsetstrokealpha(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd

drw_wnd => rdi
stroke_opacity => xmm0
*/

void fh_drawsetstrokealpha(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd");

/*
void HPHP::f_drawsetstrokeantialias(HPHP::Object const&, bool)
_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb

drw_wnd => rdi
stroke_antialias => rsi
*/

void fh_drawsetstrokeantialias(Value* drw_wnd, bool stroke_antialias) asm("_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb");

/*
void HPHP::f_drawsetstrokecolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_

drw_wnd => rdi
strokecolor_pxl_wnd => rsi
*/

void fh_drawsetstrokecolor(Value* drw_wnd, Value* strokecolor_pxl_wnd) asm("_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_");

/*
void HPHP::f_drawsetstrokedasharray(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
dash_array => rsi
*/

void fh_drawsetstrokedasharray(Value* drw_wnd, Value* dash_array) asm("_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE");

/*
void HPHP::f_drawsetstrokedashoffset(HPHP::Object const&, double)
_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd

drw_wnd => rdi
dash_offset => xmm0
*/

void fh_drawsetstrokedashoffset(Value* drw_wnd, double dash_offset) asm("_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd");

/*
void HPHP::f_drawsetstrokelinecap(HPHP::Object const&, int)
_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi

drw_wnd => rdi
line_cap => rsi
*/

void fh_drawsetstrokelinecap(Value* drw_wnd, int line_cap) asm("_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi");

/*
void HPHP::f_drawsetstrokelinejoin(HPHP::Object const&, int)
_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi

drw_wnd => rdi
line_join => rsi
*/

void fh_drawsetstrokelinejoin(Value* drw_wnd, int line_join) asm("_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi");

/*
void HPHP::f_drawsetstrokemiterlimit(HPHP::Object const&, double)
_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd

drw_wnd => rdi
miterlimit => xmm0
*/

void fh_drawsetstrokemiterlimit(Value* drw_wnd, double miterlimit) asm("_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd");

/*
void HPHP::f_drawsetstrokeopacity(HPHP::Object const&, double)
_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd

drw_wnd => rdi
stroke_opacity => xmm0
*/

void fh_drawsetstrokeopacity(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd");

/*
bool HPHP::f_drawsetstrokepatternurl(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
stroke_url => rsi
*/

bool fh_drawsetstrokepatternurl(Value* drw_wnd, Value* stroke_url) asm("_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsetstrokewidth(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd

drw_wnd => rdi
stroke_width => xmm0
*/

void fh_drawsetstrokewidth(Value* drw_wnd, double stroke_width) asm("_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd");

/*
void HPHP::f_drawsettextalignment(HPHP::Object const&, int)
_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi

drw_wnd => rdi
align_type => rsi
*/

void fh_drawsettextalignment(Value* drw_wnd, int align_type) asm("_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi");

/*
void HPHP::f_drawsettextantialias(HPHP::Object const&, bool)
_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb

drw_wnd => rdi
text_antialias => rsi
*/

void fh_drawsettextantialias(Value* drw_wnd, bool text_antialias) asm("_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb");

/*
void HPHP::f_drawsettextdecoration(HPHP::Object const&, int)
_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi

drw_wnd => rdi
decoration_type => rsi
*/

void fh_drawsettextdecoration(Value* drw_wnd, int decoration_type) asm("_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi");

/*
void HPHP::f_drawsettextencoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
encoding => rsi
*/

void fh_drawsettextencoding(Value* drw_wnd, Value* encoding) asm("_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsettextundercolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_

drw_wnd => rdi
undercolor_pxl_wnd => rsi
*/

void fh_drawsettextundercolor(Value* drw_wnd, Value* undercolor_pxl_wnd) asm("_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_");

/*
bool HPHP::f_drawsetvectorgraphics(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
vector_graphics => rsi
*/

bool fh_drawsetvectorgraphics(Value* drw_wnd, Value* vector_graphics) asm("_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawsetviewbox(HPHP::Object const&, double, double, double, double)
_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
*/

void fh_drawsetviewbox(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd");

/*
void HPHP::f_drawskewx(HPHP::Object const&, double)
_ZN4HPHP11f_drawskewxERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawskewx(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewxERKNS_6ObjectEd");

/*
void HPHP::f_drawskewy(HPHP::Object const&, double)
_ZN4HPHP11f_drawskewyERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawskewy(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewyERKNS_6ObjectEd");

/*
void HPHP::f_drawtranslate(HPHP::Object const&, double, double)
_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawtranslate(Value* drw_wnd, double x, double y) asm("_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd");

/*
void HPHP::f_pushdrawingwand(HPHP::Object const&)
_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_pushdrawingwand(Value* drw_wnd) asm("_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE");

/*
void HPHP::f_drawpushclippath(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
clip_path_id => rsi
*/

void fh_drawpushclippath(Value* drw_wnd, Value* clip_path_id) asm("_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_drawpushdefs(HPHP::Object const&)
_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpushdefs(Value* drw_wnd) asm("_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE");

/*
void HPHP::f_drawpushpattern(HPHP::Object const&, HPHP::String const&, double, double, double, double)
_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd

drw_wnd => rdi
pattern_id => rsi
x => xmm0
y => xmm1
width => xmm2
height => xmm3
*/

void fh_drawpushpattern(Value* drw_wnd, Value* pattern_id, double x, double y, double width, double height) asm("_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd");

/*
void HPHP::f_popdrawingwand(HPHP::Object const&)
_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_popdrawingwand(Value* drw_wnd) asm("_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE");

/*
void HPHP::f_drawpopclippath(HPHP::Object const&)
_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpopclippath(Value* drw_wnd) asm("_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE");

/*
void HPHP::f_drawpopdefs(HPHP::Object const&)
_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpopdefs(Value* drw_wnd) asm("_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE");

/*
void HPHP::f_drawpoppattern(HPHP::Object const&)
_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpoppattern(Value* drw_wnd) asm("_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE");

/*
bool HPHP::f_magickadaptivethresholdimage(HPHP::Object const&, double, double, double)
_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
offset => xmm2
*/

bool fh_magickadaptivethresholdimage(Value* mgck_wnd, double width, double height, double offset) asm("_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd");

/*
bool HPHP::f_magickaddimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
add_wand => rsi
*/

bool fh_magickaddimage(Value* mgck_wnd, Value* add_wand) asm("_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickaddnoiseimage(HPHP::Object const&, int)
_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
noise_type => rsi
*/

bool fh_magickaddnoiseimage(Value* mgck_wnd, int noise_type) asm("_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi");

/*
bool HPHP::f_magickaffinetransformimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
*/

bool fh_magickaffinetransformimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickannotateimage(HPHP::Object const&, HPHP::Object const&, double, double, double, HPHP::String const&)
_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
x => xmm0
y => xmm1
angle => xmm2
text => rdx
*/

bool fh_magickannotateimage(Value* mgck_wnd, Value* drw_wnd, double x, double y, double angle, Value* text) asm("_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE");

/*
HPHP::Object HPHP::f_magickappendimages(HPHP::Object const&, bool)
_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb

(return value) => rax
_rv => rdi
mgck_wnd => rsi
stack_vertical => rdx
*/

Value* fh_magickappendimages(Value* _rv, Value* mgck_wnd, bool stack_vertical) asm("_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb");

/*
HPHP::Object HPHP::f_magickaverageimages(HPHP::Object const&)
_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickaverageimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE");

/*
bool HPHP::f_magickblackthresholdimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
threshold_pxl_wnd => rsi
*/

bool fh_magickblackthresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickblurimage(HPHP::Object const&, double, double, int)
_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magickblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi");

/*
bool HPHP::f_magickborderimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
bordercolor => rsi
width => xmm0
height => xmm1
*/

bool fh_magickborderimage(Value* mgck_wnd, Value* bordercolor, double width, double height) asm("_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd");

/*
bool HPHP::f_magickcharcoalimage(HPHP::Object const&, double, double)
_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
*/

bool fh_magickcharcoalimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd");

/*
bool HPHP::f_magickchopimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickchopimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii");

/*
bool HPHP::f_magickclipimage(HPHP::Object const&)
_ZN4HPHP17f_magickclipimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickclipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickclipimageERKNS_6ObjectE");

/*
bool HPHP::f_magickclippathimage(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb

(return value) => rax
mgck_wnd => rdi
pathname => rsi
inside => rdx
*/

bool fh_magickclippathimage(Value* mgck_wnd, Value* pathname, bool inside) asm("_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb");

/*
HPHP::Object HPHP::f_magickcoalesceimages(HPHP::Object const&)
_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickcoalesceimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE");

/*
bool HPHP::f_magickcolorfloodfillimage(HPHP::Object const&, HPHP::Object const&, double, HPHP::Object const&, int, int)
_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii

(return value) => rax
mgck_wnd => rdi
fillcolor_pxl_wnd => rsi
fuzz => xmm0
bordercolor_pxl_wnd => rdx
x => rcx
y => r8
*/

bool fh_magickcolorfloodfillimage(Value* mgck_wnd, Value* fillcolor_pxl_wnd, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii");

/*
bool HPHP::f_magickcolorizeimage(HPHP::Object const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_

(return value) => rax
mgck_wnd => rdi
colorize => rsi
opacity_pxl_wnd => rdx
*/

bool fh_magickcolorizeimage(Value* mgck_wnd, Value* colorize, Value* opacity_pxl_wnd) asm("_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_");

/*
HPHP::Object HPHP::f_magickcombineimages(HPHP::Object const&, int)
_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickcombineimages(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi");

/*
bool HPHP::f_magickcommentimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
comment => rsi
*/

bool fh_magickcommentimage(Value* mgck_wnd, Value* comment) asm("_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Array HPHP::f_magickcompareimages(HPHP::Object const&, HPHP::Object const&, int, int)
_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii

(return value) => rax
_rv => rdi
mgck_wnd => rsi
reference_wnd => rdx
metric_type => rcx
channel_type => r8
*/

Value* fh_magickcompareimages(Value* _rv, Value* mgck_wnd, Value* reference_wnd, int metric_type, int channel_type) asm("_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii");

/*
bool HPHP::f_magickcompositeimage(HPHP::Object const&, HPHP::Object const&, int, int, int)
_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii

(return value) => rax
mgck_wnd => rdi
composite_wnd => rsi
composite_operator => rdx
x => rcx
y => r8
*/

bool fh_magickcompositeimage(Value* mgck_wnd, Value* composite_wnd, int composite_operator, int x, int y) asm("_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii");

/*
bool HPHP::f_magickconstituteimage(HPHP::Object const&, double, double, HPHP::String const&, int, HPHP::Array const&)
_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
smap => rsi
storage_type => rdx
pixel_array => rcx
*/

bool fh_magickconstituteimage(Value* mgck_wnd, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE");

/*
bool HPHP::f_magickcontrastimage(HPHP::Object const&, bool)
_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb

(return value) => rax
mgck_wnd => rdi
sharpen => rsi
*/

bool fh_magickcontrastimage(Value* mgck_wnd, bool sharpen) asm("_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb");

/*
bool HPHP::f_magickconvolveimage(HPHP::Object const&, HPHP::Array const&, int)
_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi

(return value) => rax
mgck_wnd => rdi
kernel_array => rsi
channel_type => rdx
*/

bool fh_magickconvolveimage(Value* mgck_wnd, Value* kernel_array, int channel_type) asm("_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi");

/*
bool HPHP::f_magickcropimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickcropimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii");

/*
bool HPHP::f_magickcyclecolormapimage(HPHP::Object const&, int)
_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
num_positions => rsi
*/

bool fh_magickcyclecolormapimage(Value* mgck_wnd, int num_positions) asm("_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi");

/*
HPHP::Object HPHP::f_magickdeconstructimages(HPHP::Object const&)
_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickdeconstructimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickdescribeimage(HPHP::Object const&)
_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickdescribeimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE");

/*
bool HPHP::f_magickdespeckleimage(HPHP::Object const&)
_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickdespeckleimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE");

/*
bool HPHP::f_magickdrawimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
*/

bool fh_magickdrawimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickechoimageblob(HPHP::Object const&)
_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickechoimageblob(Value* mgck_wnd) asm("_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE");

/*
bool HPHP::f_magickechoimagesblob(HPHP::Object const&)
_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickechoimagesblob(Value* mgck_wnd) asm("_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE");

/*
bool HPHP::f_magickedgeimage(HPHP::Object const&, double)
_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickedgeimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickembossimage(HPHP::Object const&, double, double)
_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
*/

bool fh_magickembossimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd");

/*
bool HPHP::f_magickenhanceimage(HPHP::Object const&)
_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickenhanceimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE");

/*
bool HPHP::f_magickequalizeimage(HPHP::Object const&)
_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickequalizeimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE");

/*
bool HPHP::f_magickevaluateimage(HPHP::Object const&, int, double, int)
_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi

(return value) => rax
mgck_wnd => rdi
evaluate_op => rsi
constant => xmm0
channel_type => rdx
*/

bool fh_magickevaluateimage(Value* mgck_wnd, int evaluate_op, double constant, int channel_type) asm("_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi");

/*
HPHP::Object HPHP::f_magickflattenimages(HPHP::Object const&)
_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickflattenimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE");

/*
bool HPHP::f_magickflipimage(HPHP::Object const&)
_ZN4HPHP17f_magickflipimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickflipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflipimageERKNS_6ObjectE");

/*
bool HPHP::f_magickflopimage(HPHP::Object const&)
_ZN4HPHP17f_magickflopimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickflopimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflopimageERKNS_6ObjectE");

/*
bool HPHP::f_magickframeimage(HPHP::Object const&, HPHP::Object const&, double, double, int, int)
_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii

(return value) => rax
mgck_wnd => rdi
matte_color => rsi
width => xmm0
height => xmm1
inner_bevel => rdx
outer_bevel => rcx
*/

bool fh_magickframeimage(Value* mgck_wnd, Value* matte_color, double width, double height, int inner_bevel, int outer_bevel) asm("_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii");

/*
HPHP::Object HPHP::f_magickfximage(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
expression => rdx
channel_type => rcx
*/

Value* fh_magickfximage(Value* _rv, Value* mgck_wnd, Value* expression, int channel_type) asm("_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_magickgammaimage(HPHP::Object const&, double, int)
_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi

(return value) => rax
mgck_wnd => rdi
gamma => xmm0
channel_type => rsi
*/

bool fh_magickgammaimage(Value* mgck_wnd, double gamma, int channel_type) asm("_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi");

/*
bool HPHP::f_magickgaussianblurimage(HPHP::Object const&, double, double, int)
_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magickgaussianblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi");

/*
double HPHP::f_magickgetcharheight(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetcharheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb");

/*
double HPHP::f_magickgetcharwidth(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetcharwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb");

/*
HPHP::Array HPHP::f_magickgetexception(HPHP::Object const&)
_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetexception(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetexceptionstring(HPHP::Object const&)
_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetexceptionstring(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE");

/*
long long HPHP::f_magickgetexceptiontype(HPHP::Object const&)
_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetexceptiontype(Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetfilename(HPHP::Object const&)
_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetfilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetformat(HPHP::Object const&)
_ZN4HPHP17f_magickgetformatERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_magickgetformatERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_magickgetimage(HPHP::Object const&)
_ZN4HPHP16f_magickgetimageERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP16f_magickgetimageERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_magickgetimagebackgroundcolor(HPHP::Object const&)
_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagebackgroundcolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetimageblob(HPHP::Object const&)
_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimageblueprimary(HPHP::Object const&)
_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageblueprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_magickgetimagebordercolor(HPHP::Object const&)
_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagebordercolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimagechannelmean(HPHP::Object const&, int)
_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickgetimagechannelmean(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi");

/*
HPHP::Object HPHP::f_magickgetimagecolormapcolor(HPHP::Object const&, double)
_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd

(return value) => rax
_rv => rdi
mgck_wnd => rsi
index => xmm0
*/

Value* fh_magickgetimagecolormapcolor(Value* _rv, Value* mgck_wnd, double index) asm("_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd");

/*
double HPHP::f_magickgetimagecolors(HPHP::Object const&)
_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagecolors(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagecolorspace(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagecolorspace(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagecompose(HPHP::Object const&)
_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagecompose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagecompression(HPHP::Object const&)
_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagecompression(Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagecompressionquality(HPHP::Object const&)
_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagecompressionquality(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagedelay(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagedelay(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagedepth(HPHP::Object const&, int)
_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi

(return value) => xmm0
mgck_wnd => rdi
channel_type => rsi
*/

double fh_magickgetimagedepth(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi");

/*
long long HPHP::f_magickgetimagedispose(HPHP::Object const&)
_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagedispose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimageextrema(HPHP::Object const&, int)
_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickgetimageextrema(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi");

/*
HPHP::String HPHP::f_magickgetimagefilename(HPHP::Object const&)
_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagefilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetimageformat(HPHP::Object const&)
_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagegamma(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagegamma(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimagegreenprimary(HPHP::Object const&)
_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagegreenprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE");

/*
double HPHP::f_magickgetimageheight(HPHP::Object const&)
_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimageheight(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimagehistogram(HPHP::Object const&)
_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagehistogram(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimageindex(HPHP::Object const&)
_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimageindex(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimageinterlacescheme(HPHP::Object const&)
_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimageinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE");

/*
double HPHP::f_magickgetimageiterations(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimageiterations(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_magickgetimagemattecolor(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagemattecolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetimagemimetype(HPHP::Object const&)
_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagemimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimagepixels(HPHP::Object const&, int, int, double, double, HPHP::String const&, int)
_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
x_offset => rdx
y_offset => rcx
columns => xmm0
rows => xmm1
smap => r8
storage_type => r9
*/

Value* fh_magickgetimagepixels(Value* _rv, Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type) asm("_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi");

/*
HPHP::String HPHP::f_magickgetimageprofile(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
name => rdx
*/

Value* fh_magickgetimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Array HPHP::f_magickgetimageredprimary(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageredprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagerenderingintent(HPHP::Object const&)
_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagerenderingintent(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimageresolution(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageresolution(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagescene(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagescene(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetimagesignature(HPHP::Object const&)
_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagesignature(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagesize(HPHP::Object const&)
_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagesize(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagetype(HPHP::Object const&)
_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagetype(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimageunits(HPHP::Object const&)
_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimageunits(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE");

/*
long long HPHP::f_magickgetimagevirtualpixelmethod(HPHP::Object const&)
_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetimagevirtualpixelmethod(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetimagewhitepoint(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagewhitepoint(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE");

/*
double HPHP::f_magickgetimagewidth(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagewidth(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickgetimagesblob(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagesblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE");

/*
long long HPHP::f_magickgetinterlacescheme(HPHP::Object const&)
_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long long fh_magickgetinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE");

/*
double HPHP::f_magickgetmaxtextadvance(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetmaxtextadvance(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb");

/*
HPHP::String HPHP::f_magickgetmimetype(HPHP::Object const&)
_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetmimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE");

/*
double HPHP::f_magickgetnumberimages(HPHP::Object const&)
_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetnumberimages(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetsamplingfactors(HPHP::Object const&)
_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetsamplingfactors(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_magickgetsize(HPHP::Object const&)
_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE");

/*
double HPHP::f_magickgetstringheight(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetstringheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb");

/*
double HPHP::f_magickgetstringwidth(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetstringwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb");

/*
double HPHP::f_magickgettextascent(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgettextascent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb");

/*
double HPHP::f_magickgettextdescent(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgettextdescent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb");

/*
HPHP::Array HPHP::f_magickgetwandsize(HPHP::Object const&)
_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetwandsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE");

/*
bool HPHP::f_magickhasnextimage(HPHP::Object const&)
_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickhasnextimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE");

/*
bool HPHP::f_magickhaspreviousimage(HPHP::Object const&)
_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickhaspreviousimage(Value* mgck_wnd) asm("_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE");

/*
bool HPHP::f_magickimplodeimage(HPHP::Object const&, double)
_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
amount => xmm0
*/

bool fh_magickimplodeimage(Value* mgck_wnd, double amount) asm("_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd");

/*
bool HPHP::f_magicklabelimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
label => rsi
*/

bool fh_magicklabelimage(Value* mgck_wnd, Value* label) asm("_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magicklevelimage(HPHP::Object const&, double, double, double, int)
_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi

(return value) => rax
mgck_wnd => rdi
black_point => xmm0
gamma => xmm1
white_point => xmm2
channel_type => rsi
*/

bool fh_magicklevelimage(Value* mgck_wnd, double black_point, double gamma, double white_point, int channel_type) asm("_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi");

/*
bool HPHP::f_magickmagnifyimage(HPHP::Object const&)
_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickmagnifyimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE");

/*
bool HPHP::f_magickmapimage(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b

(return value) => rax
mgck_wnd => rdi
map_wand => rsi
dither => rdx
*/

bool fh_magickmapimage(Value* mgck_wnd, Value* map_wand, bool dither) asm("_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b");

/*
bool HPHP::f_magickmattefloodfillimage(HPHP::Object const&, double, double, HPHP::Object const&, int, int)
_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii

(return value) => rax
mgck_wnd => rdi
opacity => xmm0
fuzz => xmm1
bordercolor_pxl_wnd => rsi
x => rdx
y => rcx
*/

bool fh_magickmattefloodfillimage(Value* mgck_wnd, double opacity, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii");

/*
bool HPHP::f_magickmedianfilterimage(HPHP::Object const&, double)
_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickmedianfilterimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickminifyimage(HPHP::Object const&)
_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickminifyimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE");

/*
bool HPHP::f_magickmodulateimage(HPHP::Object const&, double, double, double)
_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
brightness => xmm0
saturation => xmm1
hue => xmm2
*/

bool fh_magickmodulateimage(Value* mgck_wnd, double brightness, double saturation, double hue) asm("_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd");

/*
HPHP::Object HPHP::f_magickmontageimage(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
drw_wnd => rdx
tile_geometry => rcx
thumbnail_geometry => r8
montage_mode => r9
frame => st0
*/

Value* fh_magickmontageimage(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* tile_geometry, Value* thumbnail_geometry, int montage_mode, Value* frame) asm("_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_");

/*
HPHP::Object HPHP::f_magickmorphimages(HPHP::Object const&, double)
_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd

(return value) => rax
_rv => rdi
mgck_wnd => rsi
number_frames => xmm0
*/

Value* fh_magickmorphimages(Value* _rv, Value* mgck_wnd, double number_frames) asm("_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd");

/*
HPHP::Object HPHP::f_magickmosaicimages(HPHP::Object const&)
_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickmosaicimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE");

/*
bool HPHP::f_magickmotionblurimage(HPHP::Object const&, double, double, double)
_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
angle => xmm2
*/

bool fh_magickmotionblurimage(Value* mgck_wnd, double radius, double sigma, double angle) asm("_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd");

/*
bool HPHP::f_magicknegateimage(HPHP::Object const&, bool, int)
_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi

(return value) => rax
mgck_wnd => rdi
only_the_gray => rsi
channel_type => rdx
*/

bool fh_magicknegateimage(Value* mgck_wnd, bool only_the_gray, int channel_type) asm("_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi");

/*
bool HPHP::f_magicknewimage(HPHP::Object const&, double, double, HPHP::String const&)
_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
imagemagick_col_str => rsi
*/

bool fh_magicknewimage(Value* mgck_wnd, double width, double height, Value* imagemagick_col_str) asm("_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE");

/*
bool HPHP::f_magicknextimage(HPHP::Object const&)
_ZN4HPHP17f_magicknextimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magicknextimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magicknextimageERKNS_6ObjectE");

/*
bool HPHP::f_magicknormalizeimage(HPHP::Object const&)
_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magicknormalizeimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE");

/*
bool HPHP::f_magickoilpaintimage(HPHP::Object const&, double)
_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickoilpaintimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickpaintopaqueimage(HPHP::Object const&, HPHP::Object const&, HPHP::Object const&, double)
_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d

(return value) => rax
mgck_wnd => rdi
target_pxl_wnd => rsi
fill_pxl_wnd => rdx
fuzz => xmm0
*/

bool fh_magickpaintopaqueimage(Value* mgck_wnd, Value* target_pxl_wnd, Value* fill_pxl_wnd, double fuzz) asm("_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d");

/*
bool HPHP::f_magickpainttransparentimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
target => rsi
opacity => xmm0
fuzz => xmm1
*/

bool fh_magickpainttransparentimage(Value* mgck_wnd, Value* target, double opacity, double fuzz) asm("_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd");

/*
bool HPHP::f_magickpingimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickpingimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magickposterizeimage(HPHP::Object const&, double, bool)
_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb

(return value) => rax
mgck_wnd => rdi
levels => xmm0
dither => rsi
*/

bool fh_magickposterizeimage(Value* mgck_wnd, double levels, bool dither) asm("_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb");

/*
HPHP::Object HPHP::f_magickpreviewimages(HPHP::Object const&, int)
_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
preview => rdx
*/

Value* fh_magickpreviewimages(Value* _rv, Value* mgck_wnd, int preview) asm("_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi");

/*
bool HPHP::f_magickpreviousimage(HPHP::Object const&)
_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickpreviousimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE");

/*
bool HPHP::f_magickprofileimage(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
mgck_wnd => rdi
name => rsi
profile => rdx
*/

bool fh_magickprofileimage(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_magickquantizeimage(HPHP::Object const&, double, int, double, bool, bool)
_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb

(return value) => rax
mgck_wnd => rdi
number_colors => xmm0
colorspace_type => rsi
treedepth => xmm1
dither => rdx
measure_error => rcx
*/

bool fh_magickquantizeimage(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb");

/*
bool HPHP::f_magickquantizeimages(HPHP::Object const&, double, int, double, bool, bool)
_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb

(return value) => rax
mgck_wnd => rdi
number_colors => xmm0
colorspace_type => rsi
treedepth => xmm1
dither => rdx
measure_error => rcx
*/

bool fh_magickquantizeimages(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb");

/*
HPHP::Array HPHP::f_magickqueryfontmetrics(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb

(return value) => rax
_rv => rdi
mgck_wnd => rsi
drw_wnd => rdx
txt => rcx
multiline => r8
*/

Value* fh_magickqueryfontmetrics(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb");

/*
bool HPHP::f_magickradialblurimage(HPHP::Object const&, double)
_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
angle => xmm0
*/

bool fh_magickradialblurimage(Value* mgck_wnd, double angle) asm("_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickraiseimage(HPHP::Object const&, double, double, int, int, bool)
_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
raise => rcx
*/

bool fh_magickraiseimage(Value* mgck_wnd, double width, double height, int x, int y, bool raise) asm("_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib");

/*
bool HPHP::f_magickreadimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickreadimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magickreadimageblob(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
blob => rsi
*/

bool fh_magickreadimageblob(Value* mgck_wnd, Value* blob) asm("_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magickreadimagefile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickreadimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_");

/*
bool HPHP::f_magickreadimages(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
img_filenames_array => rsi
*/

bool fh_magickreadimages(Value* mgck_wnd, Value* img_filenames_array) asm("_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE");

/*
bool HPHP::f_magickreducenoiseimage(HPHP::Object const&, double)
_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickreducenoiseimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickremoveimage(HPHP::Object const&)
_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickremoveimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE");

/*
HPHP::String HPHP::f_magickremoveimageprofile(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
name => rdx
*/

Value* fh_magickremoveimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magickremoveimageprofiles(HPHP::Object const&)
_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickremoveimageprofiles(Value* mgck_wnd) asm("_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE");

/*
bool HPHP::f_magickresampleimage(HPHP::Object const&, double, double, int, double)
_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
filter_type => rsi
blur => xmm2
*/

bool fh_magickresampleimage(Value* mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) asm("_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid");

/*
void HPHP::f_magickresetiterator(HPHP::Object const&)
_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magickresetiterator(Value* mgck_wnd) asm("_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE");

/*
bool HPHP::f_magickresizeimage(HPHP::Object const&, double, double, int, double)
_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
filter_type => rsi
blur => xmm2
*/

bool fh_magickresizeimage(Value* mgck_wnd, double columns, double rows, int filter_type, double blur) asm("_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid");

/*
bool HPHP::f_magickrollimage(HPHP::Object const&, int, int)
_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
x_offset => rsi
y_offset => rdx
*/

bool fh_magickrollimage(Value* mgck_wnd, int x_offset, int y_offset) asm("_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii");

/*
bool HPHP::f_magickrotateimage(HPHP::Object const&, HPHP::Object const&, double)
_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d

(return value) => rax
mgck_wnd => rdi
background => rsi
degrees => xmm0
*/

bool fh_magickrotateimage(Value* mgck_wnd, Value* background, double degrees) asm("_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d");

/*
bool HPHP::f_magicksampleimage(HPHP::Object const&, double, double)
_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
*/

bool fh_magicksampleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd");

/*
bool HPHP::f_magickscaleimage(HPHP::Object const&, double, double)
_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
*/

bool fh_magickscaleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd");

/*
bool HPHP::f_magickseparateimagechannel(HPHP::Object const&, int)
_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
channel_type => rsi
*/

bool fh_magickseparateimagechannel(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetcompressionquality(HPHP::Object const&, double)
_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
quality => xmm0
*/

bool fh_magicksetcompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetfilename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magicksetfilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_magicksetfirstiterator(HPHP::Object const&)
_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magicksetfirstiterator(Value* mgck_wnd) asm("_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE");

/*
bool HPHP::f_magicksetformat(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
format => rsi
*/

bool fh_magicksetformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magicksetimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
replace_wand => rsi
*/

bool fh_magicksetimage(Value* mgck_wnd, Value* replace_wand) asm("_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magicksetimagebackgroundcolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
background_pxl_wnd => rsi
*/

bool fh_magicksetimagebackgroundcolor(Value* mgck_wnd, Value* background_pxl_wnd) asm("_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_");

/*
bool HPHP::f_magicksetimagebias(HPHP::Object const&, double)
_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
bias => xmm0
*/

bool fh_magicksetimagebias(Value* mgck_wnd, double bias) asm("_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimageblueprimary(HPHP::Object const&, double, double)
_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimageblueprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetimagebordercolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
border_pxl_wnd => rsi
*/

bool fh_magicksetimagebordercolor(Value* mgck_wnd, Value* border_pxl_wnd) asm("_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_");

/*
bool HPHP::f_magicksetimagecolormapcolor(HPHP::Object const&, double, HPHP::Object const&)
_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_

(return value) => rax
mgck_wnd => rdi
index => xmm0
mapcolor_pxl_wnd => rsi
*/

bool fh_magicksetimagecolormapcolor(Value* mgck_wnd, double index, Value* mapcolor_pxl_wnd) asm("_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_");

/*
bool HPHP::f_magicksetimagecolorspace(HPHP::Object const&, int)
_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
colorspace_type => rsi
*/

bool fh_magicksetimagecolorspace(Value* mgck_wnd, int colorspace_type) asm("_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagecompose(HPHP::Object const&, int)
_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
composite_operator => rsi
*/

bool fh_magicksetimagecompose(Value* mgck_wnd, int composite_operator) asm("_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagecompression(HPHP::Object const&, int)
_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
compression_type => rsi
*/

bool fh_magicksetimagecompression(Value* mgck_wnd, int compression_type) asm("_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagecompressionquality(HPHP::Object const&, double)
_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
quality => xmm0
*/

bool fh_magicksetimagecompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimagedelay(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
delay => xmm0
*/

bool fh_magicksetimagedelay(Value* mgck_wnd, double delay) asm("_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimagedepth(HPHP::Object const&, int, int)
_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
depth => rsi
channel_type => rdx
*/

bool fh_magicksetimagedepth(Value* mgck_wnd, int depth, int channel_type) asm("_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii");

/*
bool HPHP::f_magicksetimagedispose(HPHP::Object const&, int)
_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
dispose_type => rsi
*/

bool fh_magicksetimagedispose(Value* mgck_wnd, int dispose_type) asm("_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagefilename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magicksetimagefilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magicksetimageformat(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
format => rsi
*/

bool fh_magicksetimageformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magicksetimagegamma(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
gamma => xmm0
*/

bool fh_magicksetimagegamma(Value* mgck_wnd, double gamma) asm("_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimagegreenprimary(HPHP::Object const&, double, double)
_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimagegreenprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetimageindex(HPHP::Object const&, int)
_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
index => rsi
*/

bool fh_magicksetimageindex(Value* mgck_wnd, int index) asm("_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimageinterlacescheme(HPHP::Object const&, int)
_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
interlace_type => rsi
*/

bool fh_magicksetimageinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimageiterations(HPHP::Object const&, double)
_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
iterations => xmm0
*/

bool fh_magicksetimageiterations(Value* mgck_wnd, double iterations) asm("_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimagemattecolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
matte_pxl_wnd => rsi
*/

bool fh_magicksetimagemattecolor(Value* mgck_wnd, Value* matte_pxl_wnd) asm("_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_");

/*
bool HPHP::f_magicksetimageoption(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
mgck_wnd => rdi
format => rsi
key => rdx
value => rcx
*/

bool fh_magicksetimageoption(Value* mgck_wnd, Value* format, Value* key, Value* value) asm("_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_magicksetimagepixels(HPHP::Object const&, int, int, double, double, HPHP::String const&, int, HPHP::Array const&)
_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
x_offset => rsi
y_offset => rdx
columns => xmm0
rows => xmm1
smap => rcx
storage_type => r8
pixel_array => r9
*/

bool fh_magicksetimagepixels(Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE");

/*
bool HPHP::f_magicksetimageprofile(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
mgck_wnd => rdi
name => rsi
profile => rdx
*/

bool fh_magicksetimageprofile(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_magicksetimageredprimary(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimageredprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetimagerenderingintent(HPHP::Object const&, int)
_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
rendering_intent => rsi
*/

bool fh_magicksetimagerenderingintent(Value* mgck_wnd, int rendering_intent) asm("_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimageresolution(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
*/

bool fh_magicksetimageresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetimagescene(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
scene => xmm0
*/

bool fh_magicksetimagescene(Value* mgck_wnd, double scene) asm("_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd");

/*
bool HPHP::f_magicksetimagetype(HPHP::Object const&, int)
_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
image_type => rsi
*/

bool fh_magicksetimagetype(Value* mgck_wnd, int image_type) asm("_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimageunits(HPHP::Object const&, int)
_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
resolution_type => rsi
*/

bool fh_magicksetimageunits(Value* mgck_wnd, int resolution_type) asm("_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagevirtualpixelmethod(HPHP::Object const&, int)
_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
virtual_pixel_method => rsi
*/

bool fh_magicksetimagevirtualpixelmethod(Value* mgck_wnd, int virtual_pixel_method) asm("_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi");

/*
bool HPHP::f_magicksetimagewhitepoint(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimagewhitepoint(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetinterlacescheme(HPHP::Object const&, int)
_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
interlace_type => rsi
*/

bool fh_magicksetinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi");

/*
void HPHP::f_magicksetlastiterator(HPHP::Object const&)
_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magicksetlastiterator(Value* mgck_wnd) asm("_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE");

/*
bool HPHP::f_magicksetpassphrase(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
passphrase => rsi
*/

bool fh_magicksetpassphrase(Value* mgck_wnd, Value* passphrase) asm("_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magicksetresolution(HPHP::Object const&, double, double)
_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
*/

bool fh_magicksetresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd");

/*
bool HPHP::f_magicksetsamplingfactors(HPHP::Object const&, double, HPHP::Array const&)
_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
number_factors => xmm0
sampling_factors => rsi
*/

bool fh_magicksetsamplingfactors(Value* mgck_wnd, double number_factors, Value* sampling_factors) asm("_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE");

/*
bool HPHP::f_magicksetsize(HPHP::Object const&, int, int)
_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magicksetsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii");

/*
bool HPHP::f_magicksetwandsize(HPHP::Object const&, int, int)
_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magicksetwandsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii");

/*
bool HPHP::f_magicksharpenimage(HPHP::Object const&, double, double, int)
_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magicksharpenimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi");

/*
bool HPHP::f_magickshaveimage(HPHP::Object const&, int, int)
_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magickshaveimage(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii");

/*
bool HPHP::f_magickshearimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
background => rsi
x_shear => xmm0
y_shear => xmm1
*/

bool fh_magickshearimage(Value* mgck_wnd, Value* background, double x_shear, double y_shear) asm("_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd");

/*
bool HPHP::f_magicksolarizeimage(HPHP::Object const&, double)
_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
threshold => xmm0
*/

bool fh_magicksolarizeimage(Value* mgck_wnd, double threshold) asm("_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickspliceimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickspliceimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii");

/*
bool HPHP::f_magickspreadimage(HPHP::Object const&, double)
_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickspreadimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd");

/*
HPHP::Object HPHP::f_magicksteganoimage(HPHP::Object const&, HPHP::Object const&, int)
_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i

(return value) => rax
_rv => rdi
mgck_wnd => rsi
watermark_wand => rdx
offset => rcx
*/

Value* fh_magicksteganoimage(Value* _rv, Value* mgck_wnd, Value* watermark_wand, int offset) asm("_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i");

/*
bool HPHP::f_magickstereoimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
offset_wand => rsi
*/

bool fh_magickstereoimage(Value* mgck_wnd, Value* offset_wand) asm("_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickstripimage(HPHP::Object const&)
_ZN4HPHP18f_magickstripimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickstripimage(Value* mgck_wnd) asm("_ZN4HPHP18f_magickstripimageERKNS_6ObjectE");

/*
bool HPHP::f_magickswirlimage(HPHP::Object const&, double)
_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
degrees => xmm0
*/

bool fh_magickswirlimage(Value* mgck_wnd, double degrees) asm("_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd");

/*
HPHP::Object HPHP::f_magicktextureimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
texture_wand => rdx
*/

Value* fh_magicktextureimage(Value* _rv, Value* mgck_wnd, Value* texture_wand) asm("_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickthresholdimage(HPHP::Object const&, double, int)
_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi

(return value) => rax
mgck_wnd => rdi
threshold => xmm0
channel_type => rsi
*/

bool fh_magickthresholdimage(Value* mgck_wnd, double threshold, int channel_type) asm("_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi");

/*
bool HPHP::f_magicktintimage(HPHP::Object const&, int, HPHP::Object const&)
_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_

(return value) => rax
mgck_wnd => rdi
tint_pxl_wnd => rsi
opacity_pxl_wnd => rdx
*/

bool fh_magicktintimage(Value* mgck_wnd, int tint_pxl_wnd, Value* opacity_pxl_wnd) asm("_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_");

/*
HPHP::Object HPHP::f_magicktransformimage(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
crop => rdx
geometry => rcx
*/

Value* fh_magicktransformimage(Value* _rv, Value* mgck_wnd, Value* crop, Value* geometry) asm("_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_magicktrimimage(HPHP::Object const&, double)
_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
fuzz => xmm0
*/

bool fh_magicktrimimage(Value* mgck_wnd, double fuzz) asm("_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd");

/*
bool HPHP::f_magickunsharpmaskimage(HPHP::Object const&, double, double, double, double, int)
_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
amount => xmm2
threshold => xmm3
channel_type => rsi
*/

bool fh_magickunsharpmaskimage(Value* mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type) asm("_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi");

/*
bool HPHP::f_magickwaveimage(HPHP::Object const&, double, double)
_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
amplitude => xmm0
wave_length => xmm1
*/

bool fh_magickwaveimage(Value* mgck_wnd, double amplitude, double wave_length) asm("_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd");

/*
bool HPHP::f_magickwhitethresholdimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
threshold_pxl_wnd => rsi
*/

bool fh_magickwhitethresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_");

/*
bool HPHP::f_magickwriteimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickwriteimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_magickwriteimagefile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickwriteimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_");

/*
bool HPHP::f_magickwriteimages(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb

(return value) => rax
mgck_wnd => rdi
filename => rsi
join_images => rdx
*/

bool fh_magickwriteimages(Value* mgck_wnd, Value* filename, bool join_images) asm("_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb");

/*
bool HPHP::f_magickwriteimagesfile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickwriteimagesfile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_");

/*
double HPHP::f_pixelgetalpha(HPHP::Object const&)
_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetalpha(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE");

/*
double HPHP::f_pixelgetalphaquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetalphaquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE");

/*
double HPHP::f_pixelgetblack(HPHP::Object const&)
_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblack(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE");

/*
double HPHP::f_pixelgetblackquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblackquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE");

/*
double HPHP::f_pixelgetblue(HPHP::Object const&)
_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblue(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE");

/*
double HPHP::f_pixelgetbluequantum(HPHP::Object const&)
_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetbluequantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE");

/*
HPHP::String HPHP::f_pixelgetcolorasstring(HPHP::Object const&)
_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetcolorasstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE");

/*
double HPHP::f_pixelgetcolorcount(HPHP::Object const&)
_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcolorcount(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE");

/*
double HPHP::f_pixelgetcyan(HPHP::Object const&)
_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcyan(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE");

/*
double HPHP::f_pixelgetcyanquantum(HPHP::Object const&)
_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcyanquantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_pixelgetexception(HPHP::Object const&)
_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetexception(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE");

/*
HPHP::String HPHP::f_pixelgetexceptionstring(HPHP::Object const&)
_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetexceptionstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE");

/*
long long HPHP::f_pixelgetexceptiontype(HPHP::Object const&)
_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE

(return value) => rax
pxl_wnd => rdi
*/

long long fh_pixelgetexceptiontype(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE");

/*
double HPHP::f_pixelgetgreen(HPHP::Object const&)
_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetgreen(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE");

/*
double HPHP::f_pixelgetgreenquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetgreenquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE");

/*
double HPHP::f_pixelgetindex(HPHP::Object const&)
_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetindex(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE");

/*
double HPHP::f_pixelgetmagenta(HPHP::Object const&)
_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetmagenta(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE");

/*
double HPHP::f_pixelgetmagentaquantum(HPHP::Object const&)
_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetmagentaquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE");

/*
double HPHP::f_pixelgetopacity(HPHP::Object const&)
_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetopacity(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE");

/*
double HPHP::f_pixelgetopacityquantum(HPHP::Object const&)
_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetopacityquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_pixelgetquantumcolor(HPHP::Object const&)
_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetquantumcolor(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE");

/*
double HPHP::f_pixelgetred(HPHP::Object const&)
_ZN4HPHP13f_pixelgetredERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetred(Value* pxl_wnd) asm("_ZN4HPHP13f_pixelgetredERKNS_6ObjectE");

/*
double HPHP::f_pixelgetredquantum(HPHP::Object const&)
_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetredquantum(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE");

/*
double HPHP::f_pixelgetyellow(HPHP::Object const&)
_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetyellow(Value* pxl_wnd) asm("_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE");

/*
double HPHP::f_pixelgetyellowquantum(HPHP::Object const&)
_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetyellowquantum(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE");

/*
void HPHP::f_pixelsetalpha(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd

pxl_wnd => rdi
alpha => xmm0
*/

void fh_pixelsetalpha(Value* pxl_wnd, double alpha) asm("_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetalphaquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd

pxl_wnd => rdi
alpha => xmm0
*/

void fh_pixelsetalphaquantum(Value* pxl_wnd, double alpha) asm("_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetblack(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd

pxl_wnd => rdi
black => xmm0
*/

void fh_pixelsetblack(Value* pxl_wnd, double black) asm("_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetblackquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd

pxl_wnd => rdi
black => xmm0
*/

void fh_pixelsetblackquantum(Value* pxl_wnd, double black) asm("_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetblue(HPHP::Object const&, double)
_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd

pxl_wnd => rdi
blue => xmm0
*/

void fh_pixelsetblue(Value* pxl_wnd, double blue) asm("_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetbluequantum(HPHP::Object const&, double)
_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd

pxl_wnd => rdi
blue => xmm0
*/

void fh_pixelsetbluequantum(Value* pxl_wnd, double blue) asm("_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetcolor(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE

pxl_wnd => rdi
imagemagick_col_str => rsi
*/

void fh_pixelsetcolor(Value* pxl_wnd, Value* imagemagick_col_str) asm("_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_pixelsetcolorcount(HPHP::Object const&, int)
_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi

pxl_wnd => rdi
count => rsi
*/

void fh_pixelsetcolorcount(Value* pxl_wnd, int count) asm("_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi");

/*
void HPHP::f_pixelsetcyan(HPHP::Object const&, double)
_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd

pxl_wnd => rdi
cyan => xmm0
*/

void fh_pixelsetcyan(Value* pxl_wnd, double cyan) asm("_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetcyanquantum(HPHP::Object const&, double)
_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd

pxl_wnd => rdi
cyan => xmm0
*/

void fh_pixelsetcyanquantum(Value* pxl_wnd, double cyan) asm("_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetgreen(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd

pxl_wnd => rdi
green => xmm0
*/

void fh_pixelsetgreen(Value* pxl_wnd, double green) asm("_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetgreenquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd

pxl_wnd => rdi
green => xmm0
*/

void fh_pixelsetgreenquantum(Value* pxl_wnd, double green) asm("_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetindex(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd

pxl_wnd => rdi
index => xmm0
*/

void fh_pixelsetindex(Value* pxl_wnd, double index) asm("_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetmagenta(HPHP::Object const&, double)
_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd

pxl_wnd => rdi
magenta => xmm0
*/

void fh_pixelsetmagenta(Value* pxl_wnd, double magenta) asm("_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetmagentaquantum(HPHP::Object const&, double)
_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd

pxl_wnd => rdi
magenta => xmm0
*/

void fh_pixelsetmagentaquantum(Value* pxl_wnd, double magenta) asm("_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetopacity(HPHP::Object const&, double)
_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd

pxl_wnd => rdi
opacity => xmm0
*/

void fh_pixelsetopacity(Value* pxl_wnd, double opacity) asm("_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetopacityquantum(HPHP::Object const&, double)
_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd

pxl_wnd => rdi
opacity => xmm0
*/

void fh_pixelsetopacityquantum(Value* pxl_wnd, double opacity) asm("_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetquantumcolor(HPHP::Object const&, double, double, double, double)
_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd

pxl_wnd => rdi
red => xmm0
green => xmm1
blue => xmm2
opacity => xmm3
*/

void fh_pixelsetquantumcolor(Value* pxl_wnd, double red, double green, double blue, double opacity) asm("_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd");

/*
void HPHP::f_pixelsetred(HPHP::Object const&, double)
_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd

pxl_wnd => rdi
red => xmm0
*/

void fh_pixelsetred(Value* pxl_wnd, double red) asm("_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetredquantum(HPHP::Object const&, double)
_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd

pxl_wnd => rdi
red => xmm0
*/

void fh_pixelsetredquantum(Value* pxl_wnd, double red) asm("_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetyellow(HPHP::Object const&, double)
_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd

pxl_wnd => rdi
yellow => xmm0
*/

void fh_pixelsetyellow(Value* pxl_wnd, double yellow) asm("_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd");

/*
void HPHP::f_pixelsetyellowquantum(HPHP::Object const&, double)
_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd

pxl_wnd => rdi
yellow => xmm0
*/

void fh_pixelsetyellowquantum(Value* pxl_wnd, double yellow) asm("_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd");

/*
HPHP::Array HPHP::f_pixelgetiteratorexception(HPHP::Object const&)
_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetiteratorexception(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE");

/*
HPHP::String HPHP::f_pixelgetiteratorexceptionstring(HPHP::Object const&)
_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetiteratorexceptionstring(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE");

/*
long long HPHP::f_pixelgetiteratorexceptiontype(HPHP::Object const&)
_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE

(return value) => rax
pxl_iter => rdi
*/

long long fh_pixelgetiteratorexceptiontype(Value* pxl_iter) asm("_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_pixelgetnextiteratorrow(HPHP::Object const&)
_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetnextiteratorrow(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE");

/*
void HPHP::f_pixelresetiterator(HPHP::Object const&)
_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_pixelresetiterator(Value* pxl_iter) asm("_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE");

/*
bool HPHP::f_pixelsetiteratorrow(HPHP::Object const&, int)
_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi

(return value) => rax
pxl_iter => rdi
row => rsi
*/

bool fh_pixelsetiteratorrow(Value* pxl_iter, int row) asm("_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi");

/*
bool HPHP::f_pixelsynciterator(HPHP::Object const&)
_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE

(return value) => rax
pxl_iter => rdi
*/

bool fh_pixelsynciterator(Value* pxl_iter) asm("_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE");


} // !HPHP

