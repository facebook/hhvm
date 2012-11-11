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
HPHP::Array HPHP::f_gd_info()
_ZN4HPHP9f_gd_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_gd_info(Value* _rv) asm("_ZN4HPHP9f_gd_infoEv");

/*
HPHP::Variant HPHP::f_getimagesize(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
filename => rsi
imageinfo => rdx
*/

TypedValue* fh_getimagesize(TypedValue* _rv, Value* filename, TypedValue* imageinfo) asm("_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::String HPHP::f_image_type_to_extension(int, bool)
_ZN4HPHP25f_image_type_to_extensionEib

(return value) => rax
_rv => rdi
imagetype => rsi
include_dot => rdx
*/

Value* fh_image_type_to_extension(Value* _rv, int imagetype, bool include_dot) asm("_ZN4HPHP25f_image_type_to_extensionEib");

/*
HPHP::String HPHP::f_image_type_to_mime_type(int)
_ZN4HPHP25f_image_type_to_mime_typeEi

(return value) => rax
_rv => rdi
imagetype => rsi
*/

Value* fh_image_type_to_mime_type(Value* _rv, int imagetype) asm("_ZN4HPHP25f_image_type_to_mime_typeEi");

/*
bool HPHP::f_image2wbmp(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
threshold => rdx
*/

bool fh_image2wbmp(Value* image, Value* filename, int threshold) asm("_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_imagealphablending(HPHP::Object const&, bool)
_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb

(return value) => rax
image => rdi
blendmode => rsi
*/

bool fh_imagealphablending(Value* image, bool blendmode) asm("_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb");

/*
bool HPHP::f_imageantialias(HPHP::Object const&, bool)
_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb

(return value) => rax
image => rdi
on => rsi
*/

bool fh_imageantialias(Value* image, bool on) asm("_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb");

/*
bool HPHP::f_imagearc(HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
start => r9
end => st0
color => st8
*/

bool fh_imagearc(Value* image, int cx, int cy, int width, int height, int start, int end, int color) asm("_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii");

/*
bool HPHP::f_imagechar(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
c => r8
color => r9
*/

bool fh_imagechar(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi");

/*
bool HPHP::f_imagecharup(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
c => r8
color => r9
*/

bool fh_imagecharup(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi");

/*
HPHP::Variant HPHP::f_imagecolorallocate(HPHP::Object const&, int, int, int)
_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorallocate(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii");

/*
HPHP::Variant HPHP::f_imagecolorallocatealpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorallocatealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii");

/*
HPHP::Variant HPHP::f_imagecolorat(HPHP::Object const&, int, int)
_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii

(return value) => rax
_rv => rdi
image => rsi
x => rdx
y => rcx
*/

TypedValue* fh_imagecolorat(TypedValue* _rv, Value* image, int x, int y) asm("_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii");

/*
HPHP::Variant HPHP::f_imagecolorclosest(HPHP::Object const&, int, int, int)
_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorclosest(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii");

/*
HPHP::Variant HPHP::f_imagecolorclosestalpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorclosestalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii");

/*
HPHP::Variant HPHP::f_imagecolorclosesthwb(HPHP::Object const&, int, int, int)
_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorclosesthwb(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii");

/*
bool HPHP::f_imagecolordeallocate(HPHP::Object const&, int)
_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi

(return value) => rax
image => rdi
color => rsi
*/

bool fh_imagecolordeallocate(Value* image, int color) asm("_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::f_imagecolorexact(HPHP::Object const&, int, int, int)
_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorexact(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii");

/*
HPHP::Variant HPHP::f_imagecolorexactalpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorexactalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii");

/*
HPHP::Variant HPHP::f_imagecolormatch(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
image1 => rsi
image2 => rdx
*/

TypedValue* fh_imagecolormatch(TypedValue* _rv, Value* image1, Value* image2) asm("_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_imagecolorresolve(HPHP::Object const&, int, int, int)
_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorresolve(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii");

/*
HPHP::Variant HPHP::f_imagecolorresolvealpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorresolvealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii");

/*
HPHP::Variant HPHP::f_imagecolorset(HPHP::Object const&, int, int, int, int)
_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
index => rdx
red => rcx
green => r8
blue => r9
*/

TypedValue* fh_imagecolorset(TypedValue* _rv, Value* image, int index, int red, int green, int blue) asm("_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii");

/*
HPHP::Variant HPHP::f_imagecolorsforindex(HPHP::Object const&, int)
_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
index => rdx
*/

TypedValue* fh_imagecolorsforindex(TypedValue* _rv, Value* image, int index) asm("_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::f_imagecolorstotal(HPHP::Object const&)
_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagecolorstotal(TypedValue* _rv, Value* image) asm("_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imagecolortransparent(HPHP::Object const&, int)
_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
color => rdx
*/

TypedValue* fh_imagecolortransparent(TypedValue* _rv, Value* image, int color) asm("_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi");

/*
bool HPHP::f_imageconvolution(HPHP::Object const&, HPHP::Array const&, double, double)
_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd

(return value) => rax
image => rdi
matrix => rsi
div => xmm0
offset => xmm1
*/

bool fh_imageconvolution(Value* image, Value* matrix, double div, double offset) asm("_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd");

/*
bool HPHP::f_imagecopy(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int)
_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
*/

bool fh_imagecopy(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h) asm("_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii");

/*
bool HPHP::f_imagecopymerge(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
pct => st16
*/

bool fh_imagecopymerge(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii");

/*
bool HPHP::f_imagecopymergegray(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
pct => st16
*/

bool fh_imagecopymergegray(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii");

/*
bool HPHP::f_imagecopyresampled(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
dst_w => st0
dst_h => st8
src_w => st16
src_h => st24
*/

bool fh_imagecopyresampled(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii");

/*
bool HPHP::f_imagecopyresized(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
dst_w => st0
dst_h => st8
src_w => st16
src_h => st24
*/

bool fh_imagecopyresized(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii");

/*
HPHP::Variant HPHP::f_imagecreate(int, int)
_ZN4HPHP13f_imagecreateEii

(return value) => rax
_rv => rdi
width => rsi
height => rdx
*/

TypedValue* fh_imagecreate(TypedValue* _rv, int width, int height) asm("_ZN4HPHP13f_imagecreateEii");

/*
HPHP::Variant HPHP::f_imagecreatefromgd2part(HPHP::String const&, int, int, int, int)
_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii

(return value) => rax
_rv => rdi
filename => rsi
srcx => rdx
srcy => rcx
width => r8
height => r9
*/

TypedValue* fh_imagecreatefromgd2part(TypedValue* _rv, Value* filename, int srcx, int srcy, int width, int height) asm("_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii");

/*
HPHP::Variant HPHP::f_imagecreatefromgd(HPHP::String const&)
_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgd(TypedValue* _rv, Value* filename) asm("_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromgd2(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgd2(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromgif(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgif(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromjpeg(HPHP::String const&)
_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromjpeg(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefrompng(HPHP::String const&)
_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefrompng(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromstring(HPHP::String const&)
_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_imagecreatefromstring(TypedValue* _rv, Value* data) asm("_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromwbmp(HPHP::String const&)
_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromwbmp(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromxbm(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromxbm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatefromxpm(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromxpm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagecreatetruecolor(int, int)
_ZN4HPHP22f_imagecreatetruecolorEii

(return value) => rax
_rv => rdi
width => rsi
height => rdx
*/

TypedValue* fh_imagecreatetruecolor(TypedValue* _rv, int width, int height) asm("_ZN4HPHP22f_imagecreatetruecolorEii");

/*
bool HPHP::f_imagedashedline(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagedashedline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii");

/*
bool HPHP::f_imagedestroy(HPHP::Object const&)
_ZN4HPHP14f_imagedestroyERKNS_6ObjectE

(return value) => rax
image => rdi
*/

bool fh_imagedestroy(Value* image) asm("_ZN4HPHP14f_imagedestroyERKNS_6ObjectE");

/*
bool HPHP::f_imageellipse(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
color => r9
*/

bool fh_imageellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii");

/*
bool HPHP::f_imagefill(HPHP::Object const&, int, int, int)
_ZN4HPHP11f_imagefillERKNS_6ObjectEiii

(return value) => rax
image => rdi
x => rsi
y => rdx
color => rcx
*/

bool fh_imagefill(Value* image, int x, int y, int color) asm("_ZN4HPHP11f_imagefillERKNS_6ObjectEiii");

/*
bool HPHP::f_imagefilledarc(HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
start => r9
end => st0
color => st8
style => st16
*/

bool fh_imagefilledarc(Value* image, int cx, int cy, int width, int height, int start, int end, int color, int style) asm("_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii");

/*
bool HPHP::f_imagefilledellipse(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
color => r9
*/

bool fh_imagefilledellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii");

/*
bool HPHP::f_imagefilledpolygon(HPHP::Object const&, HPHP::Array const&, int, int)
_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii

(return value) => rax
image => rdi
points => rsi
num_points => rdx
color => rcx
*/

bool fh_imagefilledpolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii");

/*
bool HPHP::f_imagefilledrectangle(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagefilledrectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii");

/*
bool HPHP::f_imagefilltoborder(HPHP::Object const&, int, int, int, int)
_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii

(return value) => rax
image => rdi
x => rsi
y => rdx
border => rcx
color => r8
*/

bool fh_imagefilltoborder(Value* image, int x, int y, int border, int color) asm("_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii");

/*
bool HPHP::f_imagefilter(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
filtertype => rsi
arg1 => rdx
arg2 => rcx
arg3 => r8
arg4 => r9
*/

bool fh_imagefilter(Value* image, int filtertype, int arg1, int arg2, int arg3, int arg4) asm("_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii");

/*
long long HPHP::f_imagefontheight(int)
_ZN4HPHP17f_imagefontheightEi

(return value) => rax
font => rdi
*/

long long fh_imagefontheight(int font) asm("_ZN4HPHP17f_imagefontheightEi");

/*
long long HPHP::f_imagefontwidth(int)
_ZN4HPHP16f_imagefontwidthEi

(return value) => rax
font => rdi
*/

long long fh_imagefontwidth(int font) asm("_ZN4HPHP16f_imagefontwidthEi");

/*
HPHP::Variant HPHP::f_imageftbbox(double, double, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
size => xmm0
angle => xmm1
font_file => rsi
text => rdx
extrainfo => rcx
*/

TypedValue* fh_imageftbbox(TypedValue* _rv, double size, double angle, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_imagefttext(HPHP::Object const&, double, double, int, int, int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE

(return value) => rax
_rv => rdi
image => rsi
size => xmm0
angle => xmm1
x => rdx
y => rcx
col => r8
font_file => r9
text => st0
extrainfo => st8
*/

TypedValue* fh_imagefttext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int col, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE");

/*
bool HPHP::f_imagegammacorrect(HPHP::Object const&, double, double)
_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd

(return value) => rax
image => rdi
inputgamma => xmm0
outputgamma => xmm1
*/

bool fh_imagegammacorrect(Value* image, double inputgamma, double outputgamma) asm("_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd");

/*
bool HPHP::f_imagegd2(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii

(return value) => rax
image => rdi
filename => rsi
chunk_size => rdx
type => rcx
*/

bool fh_imagegd2(Value* image, Value* filename, int chunk_size, int type) asm("_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii");

/*
bool HPHP::f_imagegd(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE

(return value) => rax
image => rdi
filename => rsi
*/

bool fh_imagegd(Value* image, Value* filename) asm("_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_imagegif(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE

(return value) => rax
image => rdi
filename => rsi
*/

bool fh_imagegif(Value* image, Value* filename) asm("_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imagegrabscreen()
_ZN4HPHP17f_imagegrabscreenEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_imagegrabscreen(TypedValue* _rv) asm("_ZN4HPHP17f_imagegrabscreenEv");

/*
HPHP::Variant HPHP::f_imagegrabwindow(int, int)
_ZN4HPHP17f_imagegrabwindowEii

(return value) => rax
_rv => rdi
window => rsi
client_area => rdx
*/

TypedValue* fh_imagegrabwindow(TypedValue* _rv, int window, int client_area) asm("_ZN4HPHP17f_imagegrabwindowEii");

/*
HPHP::Variant HPHP::f_imageinterlace(HPHP::Object const&, int)
_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
interlace => rdx
*/

TypedValue* fh_imageinterlace(TypedValue* _rv, Value* image, int interlace) asm("_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi");

/*
bool HPHP::f_imageistruecolor(HPHP::Object const&)
_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE

(return value) => rax
image => rdi
*/

bool fh_imageistruecolor(Value* image) asm("_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE");

/*
bool HPHP::f_imagejpeg(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
quality => rdx
*/

bool fh_imagejpeg(Value* image, Value* filename, int quality) asm("_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_imagelayereffect(HPHP::Object const&, int)
_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi

(return value) => rax
image => rdi
effect => rsi
*/

bool fh_imagelayereffect(Value* image, int effect) asm("_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi");

/*
bool HPHP::f_imageline(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imageline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii");

/*
HPHP::Variant HPHP::f_imageloadfont(HPHP::String const&)
_ZN4HPHP15f_imageloadfontERKNS_6StringE

(return value) => rax
_rv => rdi
file => rsi
*/

TypedValue* fh_imageloadfont(TypedValue* _rv, Value* file) asm("_ZN4HPHP15f_imageloadfontERKNS_6StringE");

/*
void HPHP::f_imagepalettecopy(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_

destination => rdi
source => rsi
*/

void fh_imagepalettecopy(Value* destination, Value* source) asm("_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_");

/*
bool HPHP::f_imagepng(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii

(return value) => rax
image => rdi
filename => rsi
quality => rdx
filters => rcx
*/

bool fh_imagepng(Value* image, Value* filename, int quality, int filters) asm("_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii");

/*
bool HPHP::f_imagepolygon(HPHP::Object const&, HPHP::Array const&, int, int)
_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii

(return value) => rax
image => rdi
points => rsi
num_points => rdx
color => rcx
*/

bool fh_imagepolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii");

/*
HPHP::Array HPHP::f_imagepsbbox(HPHP::String const&, int, int, int, int, double)
_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid

(return value) => rax
_rv => rdi
text => rsi
font => rdx
size => rcx
space => r8
tightness => r9
angle => xmm0
*/

Value* fh_imagepsbbox(Value* _rv, Value* text, int font, int size, int space, int tightness, double angle) asm("_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid");

/*
bool HPHP::f_imagepsencodefont(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE

(return value) => rax
font_index => rdi
encodingfile => rsi
*/

bool fh_imagepsencodefont(Value* font_index, Value* encodingfile) asm("_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_imagepsextendfont(int, double)
_ZN4HPHP19f_imagepsextendfontEid

(return value) => rax
font_index => rdi
extend => xmm0
*/

bool fh_imagepsextendfont(int font_index, double extend) asm("_ZN4HPHP19f_imagepsextendfontEid");

/*
bool HPHP::f_imagepsfreefont(HPHP::Object const&)
_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE

(return value) => rax
fontindex => rdi
*/

bool fh_imagepsfreefont(Value* fontindex) asm("_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_imagepsloadfont(HPHP::String const&)
_ZN4HPHP17f_imagepsloadfontERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

Value* fh_imagepsloadfont(Value* _rv, Value* filename) asm("_ZN4HPHP17f_imagepsloadfontERKNS_6StringE");

/*
bool HPHP::f_imagepsslantfont(HPHP::Object const&, double)
_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd

(return value) => rax
font_index => rdi
slant => xmm0
*/

bool fh_imagepsslantfont(Value* font_index, double slant) asm("_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd");

/*
HPHP::Array HPHP::f_imagepstext(HPHP::Object const&, HPHP::String const&, HPHP::Object const&, int, int, int, int, int, int, int, double, int)
_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi

(return value) => rax
_rv => rdi
image => rsi
text => rdx
font => rcx
size => r8
foreground => r9
background => st0
x => st8
y => st16
space => st24
tightness => st32
angle => xmm0
antialias_steps => st40
*/

Value* fh_imagepstext(Value* _rv, Value* image, Value* text, Value* font, int size, int foreground, int background, int x, int y, int space, int tightness, double angle, int antialias_steps) asm("_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi");

/*
bool HPHP::f_imagerectangle(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagerectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii");

/*
HPHP::Variant HPHP::f_imagerotate(HPHP::Object const&, double, int, int)
_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii

(return value) => rax
_rv => rdi
source_image => rsi
angle => xmm0
bgd_color => rdx
ignore_transparent => rcx
*/

TypedValue* fh_imagerotate(TypedValue* _rv, Value* source_image, double angle, int bgd_color, int ignore_transparent) asm("_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii");

/*
bool HPHP::f_imagesavealpha(HPHP::Object const&, bool)
_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb

(return value) => rax
image => rdi
saveflag => rsi
*/

bool fh_imagesavealpha(Value* image, bool saveflag) asm("_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb");

/*
bool HPHP::f_imagesetbrush(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_

(return value) => rax
image => rdi
brush => rsi
*/

bool fh_imagesetbrush(Value* image, Value* brush) asm("_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_");

/*
bool HPHP::f_imagesetpixel(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii

(return value) => rax
image => rdi
x => rsi
y => rdx
color => rcx
*/

bool fh_imagesetpixel(Value* image, int x, int y, int color) asm("_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii");

/*
bool HPHP::f_imagesetstyle(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
image => rdi
style => rsi
*/

bool fh_imagesetstyle(Value* image, Value* style) asm("_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE");

/*
bool HPHP::f_imagesetthickness(HPHP::Object const&, int)
_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi

(return value) => rax
image => rdi
thickness => rsi
*/

bool fh_imagesetthickness(Value* image, int thickness) asm("_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi");

/*
bool HPHP::f_imagesettile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_

(return value) => rax
image => rdi
tile => rsi
*/

bool fh_imagesettile(Value* image, Value* tile) asm("_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_");

/*
bool HPHP::f_imagestring(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
str => r8
color => r9
*/

bool fh_imagestring(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi");

/*
bool HPHP::f_imagestringup(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
str => r8
color => r9
*/

bool fh_imagestringup(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi");

/*
HPHP::Variant HPHP::f_imagesx(HPHP::Object const&)
_ZN4HPHP9f_imagesxERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagesx(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesxERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imagesy(HPHP::Object const&)
_ZN4HPHP9f_imagesyERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagesy(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesyERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imagetruecolortopalette(HPHP::Object const&, bool, int)
_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi

(return value) => rax
_rv => rdi
image => rsi
dither => rdx
ncolors => rcx
*/

TypedValue* fh_imagetruecolortopalette(TypedValue* _rv, Value* image, bool dither, int ncolors) asm("_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi");

/*
HPHP::Variant HPHP::f_imagettfbbox(double, double, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_

(return value) => rax
_rv => rdi
size => xmm0
angle => xmm1
fontfile => rsi
text => rdx
*/

TypedValue* fh_imagettfbbox(TypedValue* _rv, double size, double angle, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_imagettftext(HPHP::Object const&, double, double, int, int, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_

(return value) => rax
_rv => rdi
image => rsi
size => xmm0
angle => xmm1
x => rdx
y => rcx
color => r8
fontfile => r9
text => st0
*/

TypedValue* fh_imagettftext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int color, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_");

/*
long long HPHP::f_imagetypes()
_ZN4HPHP12f_imagetypesEv

(return value) => rax
*/

long long fh_imagetypes() asm("_ZN4HPHP12f_imagetypesEv");

/*
bool HPHP::f_imagewbmp(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
foreground => rdx
*/

bool fh_imagewbmp(Value* image, Value* filename, int foreground) asm("_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_imagexbm(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
foreground => rdx
*/

bool fh_imagexbm(Value* image, Value* filename, int foreground) asm("_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_iptcembed(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11f_iptcembedERKNS_6StringES2_i

(return value) => rax
_rv => rdi
iptcdata => rsi
jpeg_file_name => rdx
spool => rcx
*/

TypedValue* fh_iptcembed(TypedValue* _rv, Value* iptcdata, Value* jpeg_file_name, int spool) asm("_ZN4HPHP11f_iptcembedERKNS_6StringES2_i");

/*
HPHP::Variant HPHP::f_iptcparse(HPHP::String const&)
_ZN4HPHP11f_iptcparseERKNS_6StringE

(return value) => rax
_rv => rdi
iptcblock => rsi
*/

TypedValue* fh_iptcparse(TypedValue* _rv, Value* iptcblock) asm("_ZN4HPHP11f_iptcparseERKNS_6StringE");

/*
bool HPHP::f_jpeg2wbmp(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii

(return value) => rax
jpegname => rdi
wbmpname => rsi
dest_height => rdx
dest_width => rcx
threshold => r8
*/

bool fh_jpeg2wbmp(Value* jpegname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii");

/*
bool HPHP::f_png2wbmp(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii

(return value) => rax
pngname => rdi
wbmpname => rsi
dest_height => rdx
dest_width => rcx
threshold => r8
*/

bool fh_png2wbmp(Value* pngname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii");

/*
HPHP::Variant HPHP::f_exif_imagetype(HPHP::String const&)
_ZN4HPHP16f_exif_imagetypeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_exif_imagetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_exif_imagetypeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_exif_read_data(HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb

(return value) => rax
_rv => rdi
filename => rsi
sections => rdx
arrays => rcx
thumbnail => r8
*/

TypedValue* fh_exif_read_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb");

/*
HPHP::Variant HPHP::f_read_exif_data(HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb

(return value) => rax
_rv => rdi
filename => rsi
sections => rdx
arrays => rcx
thumbnail => r8
*/

TypedValue* fh_read_exif_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb");

/*
HPHP::Variant HPHP::f_exif_tagname(int)
_ZN4HPHP14f_exif_tagnameEi

(return value) => rax
_rv => rdi
index => rsi
*/

TypedValue* fh_exif_tagname(TypedValue* _rv, int index) asm("_ZN4HPHP14f_exif_tagnameEi");

/*
HPHP::Variant HPHP::f_exif_thumbnail(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_

(return value) => rax
_rv => rdi
filename => rsi
width => rdx
height => rcx
imagetype => r8
*/

TypedValue* fh_exif_thumbnail(TypedValue* _rv, Value* filename, TypedValue* width, TypedValue* height, TypedValue* imagetype) asm("_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_");


} // !HPHP

