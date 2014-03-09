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

#include "hphp/runtime/ext/imagick/ext_imagick.h"

namespace HPHP {

#define IMAGICKDRAW_THROW imagickThrow<ImagickDrawException>

using CUCString = const unsigned char*;

//////////////////////////////////////////////////////////////////////////////
// class ImagickDraw
static bool HHVM_METHOD(ImagickDraw, affine, CArrRef affine) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, annotation,
    double x, double y, const String& text) {
  auto wand = getDrawingWandResource(this_);
  DrawAnnotation(wand->getWand(), x, y, (CUCString)text.c_str());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, arc,
    double sx, double sy, double ex, double ey, double sd, double ed) {
  auto wand = getDrawingWandResource(this_);
  DrawArc(wand->getWand(), sx, sy, ex, ey, sd, ed);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, bezier,
    CArrRef coordinates) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, circle,
    double ox, double oy, double px, double py) {
  auto wand = getDrawingWandResource(this_);
  DrawCircle(wand->getWand(), ox, oy, px, py);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, clear) {
  auto wand = getDrawingWandResource(this_);
  ClearDrawingWand(wand->getWand());
  return true;
}

static Object HHVM_METHOD(ImagickDraw, __clone) {
  auto wand = getDrawingWandResource(this_);
  raiseDeprecated(s_ImagickDraw.c_str(), "clone");
  auto newWand = CloneDrawingWand(wand->getWand());
  if (newWand == nullptr) {
    IMAGICKDRAW_THROW("Failed to allocate DrawingWand structure");
  } else {
    auto ret = ImagickDraw::allocObject();
    setWandResource(s_ImagickDraw, ret, newWand);
    return ret;
  }
}

static bool HHVM_METHOD(ImagickDraw, color,
    double x, double y, int64_t paintMethod) {
  auto wand = getDrawingWandResource(this_);
  DrawColor(wand->getWand(), x, y, (PaintMethod)paintMethod);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, comment,
    const String& comment) {
  auto wand = getDrawingWandResource(this_);
  DrawComment(wand->getWand(), comment.c_str());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, composite,
    int64_t compose, double x, double y,
    double width, double height, CObjRef compositeWand) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static void HHVM_METHOD(ImagickDraw, __construct) {
  auto wand = NewDrawingWand();
  if (wand == nullptr) {
    IMAGICKDRAW_THROW("Failed to create ImagickDraw object");
  } else {
    setWandResource(s_ImagickDraw, this_, wand);
  }
}

static bool HHVM_METHOD(ImagickDraw, destroy) {
  return HHVM_MN(ImagickDraw, clear)(this_);
}

static bool HHVM_METHOD(ImagickDraw, ellipse,
    double ox, double oy, double rx, double ry, double start, double end) {
  auto wand = getDrawingWandResource(this_);
  DrawEllipse(wand->getWand(), ox, oy, rx, ry, start, end);
  return true;
}

static String HHVM_METHOD(ImagickDraw, getClipPath) {
  auto wand = getDrawingWandResource(this_);
  return convertMagickString(DrawGetClipPath(wand->getWand()));
}

static int64_t HHVM_METHOD(ImagickDraw, getClipRule) {
  auto wand = getDrawingWandResource(this_);

  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getClipUnits) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static Object HHVM_METHOD(ImagickDraw, getFillColor) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static double HHVM_METHOD(ImagickDraw, getFillOpacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getFillRule) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static String HHVM_METHOD(ImagickDraw, getFont) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static String HHVM_METHOD(ImagickDraw, getFontFamily) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static double HHVM_METHOD(ImagickDraw, getFontSize) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getFontStyle) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getFontWeight) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getGravity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, getStrokeAntialias) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static Object HHVM_METHOD(ImagickDraw, getStrokeColor) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static Array HHVM_METHOD(ImagickDraw, getStrokeDashArray) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static double HHVM_METHOD(ImagickDraw, getStrokeDashOffset) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeLineCap) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeLineJoin) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeMiterLimit) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static double HHVM_METHOD(ImagickDraw, getStrokeOpacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static double HHVM_METHOD(ImagickDraw, getStrokeWidth) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getTextAlignment) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, getTextAntialias) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static int64_t HHVM_METHOD(ImagickDraw, getTextDecoration) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static String HHVM_METHOD(ImagickDraw, getTextEncoding) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static Object HHVM_METHOD(ImagickDraw, getTextUnderColor) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static String HHVM_METHOD(ImagickDraw, getVectorGraphics) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, line,
    double sx, double sy, double ex, double ey) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, matte,
    double x, double y, int64_t paintMethod) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathClose) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToAbsolute,
    double x1, double y1, double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierAbsolute,
    double x1, double y1, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierRelative,
    double x1, double y1, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierSmoothAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierSmoothRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToRelative,
    double x1, double y1, double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToSmoothAbsolute,
    double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToSmoothRelative,
    double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathEllipticArcAbsolute,
    double rx, double ry, double x_axis_rotation,
    bool large_arc_flag, bool sweep_flag, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathEllipticArcRelative,
    double rx, double ry, double x_axis_rotation,
    bool large_arc_flag, bool sweep_flag, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathFinish) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToHorizontalAbsolute, double x) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToHorizontalRelative, double x) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToVerticalAbsolute, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathLineToVerticalRelative, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathMoveToAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathMoveToRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pathStart) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, point,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, polygon,
    CArrRef coordinates) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, polyline,
    CArrRef coordinates) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pop) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, popClipPath) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, popDefs) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, popPattern) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, push) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pushClipPath,
    const String& clip_mask_id) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pushDefs) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, pushPattern,
    const String& pattern_id, double x, double y, double width, double height) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, rectangle,
    double x1, double y1, double x2, double y2) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, render) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, rotate, double degrees) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, roundRectangle,
    double x1, double y1, double x2, double y2, double rx, double ry) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, scale,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setClipPath,
    const String& clip_mask) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setClipRule,
    int64_t fill_rule) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setClipUnits,
    int64_t clip_units) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFillAlpha, double opacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFillColor,
    CObjRef fill_pixel) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFillOpacity,
    double fillOpacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFillPatternURL,
    const String& fill_url) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFillRule,
    int64_t fill_rule) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFont,
    const String& font_name) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFontFamily,
    const String& font_family) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFontSize, double pointsize) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFontStretch,
    int64_t fontStretch) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFontStyle, int64_t style) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setFontWeight,
    int64_t font_weight) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setGravity, int64_t gravity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeAlpha, double opacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeAntialias,
    bool stroke_antialias) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeColor,
    CObjRef stroke_pixel) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeDashArray,
    CArrRef dashArray) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeDashOffset,
    double dash_offset) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeLineCap, int64_t linecap) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeLineJoin, int64_t linejoin) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeMiterLimit,
    int64_t miterlimit) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeOpacity,
    double stroke_opacity) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokePatternURL,
    const String& stroke_url) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setStrokeWidth,
    double stroke_width) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setTextAlignment,
    int64_t alignment) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setTextAntialias, bool antiAlias) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setTextDecoration,
    int64_t decoration) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setTextEncoding,
    const String& encoding) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setTextUnderColor,
    CObjRef under_color) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setVectorGraphics,
    const String& xml) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, setViewbox,
    int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, skewX, double degrees) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, skewY, double degrees) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

static bool HHVM_METHOD(ImagickDraw, translate,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  (void)wand;
  throw NotImplementedException("Not Implemented");
}

#undef IMAGICKDRAW_THROW

void loadImagickDrawClass() {
  HHVM_ME(ImagickDraw, affine);
  HHVM_ME(ImagickDraw, annotation);
  HHVM_ME(ImagickDraw, arc);
  HHVM_ME(ImagickDraw, bezier);
  HHVM_ME(ImagickDraw, circle);
  HHVM_ME(ImagickDraw, clear);
  HHVM_ME(ImagickDraw, __clone);
  HHVM_ME(ImagickDraw, color);
  HHVM_ME(ImagickDraw, comment);
  HHVM_ME(ImagickDraw, composite);
  HHVM_ME(ImagickDraw, __construct);
  HHVM_ME(ImagickDraw, destroy);
  HHVM_ME(ImagickDraw, ellipse);
  HHVM_ME(ImagickDraw, getClipPath);
  HHVM_ME(ImagickDraw, getClipRule);
  HHVM_ME(ImagickDraw, getClipUnits);
  HHVM_ME(ImagickDraw, getFillColor);
  HHVM_ME(ImagickDraw, getFillOpacity);
  HHVM_ME(ImagickDraw, getFillRule);
  HHVM_ME(ImagickDraw, getFont);
  HHVM_ME(ImagickDraw, getFontFamily);
  HHVM_ME(ImagickDraw, getFontSize);
  HHVM_ME(ImagickDraw, getFontStyle);
  HHVM_ME(ImagickDraw, getFontWeight);
  HHVM_ME(ImagickDraw, getGravity);
  HHVM_ME(ImagickDraw, getStrokeAntialias);
  HHVM_ME(ImagickDraw, getStrokeColor);
  HHVM_ME(ImagickDraw, getStrokeDashArray);
  HHVM_ME(ImagickDraw, getStrokeDashOffset);
  HHVM_ME(ImagickDraw, getStrokeLineCap);
  HHVM_ME(ImagickDraw, getStrokeLineJoin);
  HHVM_ME(ImagickDraw, getStrokeMiterLimit);
  HHVM_ME(ImagickDraw, getStrokeOpacity);
  HHVM_ME(ImagickDraw, getStrokeWidth);
  HHVM_ME(ImagickDraw, getTextAlignment);
  HHVM_ME(ImagickDraw, getTextAntialias);
  HHVM_ME(ImagickDraw, getTextDecoration);
  HHVM_ME(ImagickDraw, getTextEncoding);
  HHVM_ME(ImagickDraw, getTextUnderColor);
  HHVM_ME(ImagickDraw, getVectorGraphics);
  HHVM_ME(ImagickDraw, line);
  HHVM_ME(ImagickDraw, matte);
  HHVM_ME(ImagickDraw, pathClose);
  HHVM_ME(ImagickDraw, pathCurveToAbsolute);
  HHVM_ME(ImagickDraw, pathCurveToQuadraticBezierAbsolute);
  HHVM_ME(ImagickDraw, pathCurveToQuadraticBezierRelative);
  HHVM_ME(ImagickDraw, pathCurveToQuadraticBezierSmoothAbsolute);
  HHVM_ME(ImagickDraw, pathCurveToQuadraticBezierSmoothRelative);
  HHVM_ME(ImagickDraw, pathCurveToRelative);
  HHVM_ME(ImagickDraw, pathCurveToSmoothAbsolute);
  HHVM_ME(ImagickDraw, pathCurveToSmoothRelative);
  HHVM_ME(ImagickDraw, pathEllipticArcAbsolute);
  HHVM_ME(ImagickDraw, pathEllipticArcRelative);
  HHVM_ME(ImagickDraw, pathFinish);
  HHVM_ME(ImagickDraw, pathLineToAbsolute);
  HHVM_ME(ImagickDraw, pathLineToHorizontalAbsolute);
  HHVM_ME(ImagickDraw, pathLineToHorizontalRelative);
  HHVM_ME(ImagickDraw, pathLineToRelative);
  HHVM_ME(ImagickDraw, pathLineToVerticalAbsolute);
  HHVM_ME(ImagickDraw, pathLineToVerticalRelative);
  HHVM_ME(ImagickDraw, pathMoveToAbsolute);
  HHVM_ME(ImagickDraw, pathMoveToRelative);
  HHVM_ME(ImagickDraw, pathStart);
  HHVM_ME(ImagickDraw, point);
  HHVM_ME(ImagickDraw, polygon);
  HHVM_ME(ImagickDraw, polyline);
  HHVM_ME(ImagickDraw, pop);
  HHVM_ME(ImagickDraw, popClipPath);
  HHVM_ME(ImagickDraw, popDefs);
  HHVM_ME(ImagickDraw, popPattern);
  HHVM_ME(ImagickDraw, push);
  HHVM_ME(ImagickDraw, pushClipPath);
  HHVM_ME(ImagickDraw, pushDefs);
  HHVM_ME(ImagickDraw, pushPattern);
  HHVM_ME(ImagickDraw, rectangle);
  HHVM_ME(ImagickDraw, render);
  HHVM_ME(ImagickDraw, rotate);
  HHVM_ME(ImagickDraw, roundRectangle);
  HHVM_ME(ImagickDraw, scale);
  HHVM_ME(ImagickDraw, setClipPath);
  HHVM_ME(ImagickDraw, setClipRule);
  HHVM_ME(ImagickDraw, setClipUnits);
  HHVM_ME(ImagickDraw, setFillAlpha);
  HHVM_ME(ImagickDraw, setFillColor);
  HHVM_ME(ImagickDraw, setFillOpacity);
  HHVM_ME(ImagickDraw, setFillPatternURL);
  HHVM_ME(ImagickDraw, setFillRule);
  HHVM_ME(ImagickDraw, setFont);
  HHVM_ME(ImagickDraw, setFontFamily);
  HHVM_ME(ImagickDraw, setFontSize);
  HHVM_ME(ImagickDraw, setFontStretch);
  HHVM_ME(ImagickDraw, setFontStyle);
  HHVM_ME(ImagickDraw, setFontWeight);
  HHVM_ME(ImagickDraw, setGravity);
  HHVM_ME(ImagickDraw, setStrokeAlpha);
  HHVM_ME(ImagickDraw, setStrokeAntialias);
  HHVM_ME(ImagickDraw, setStrokeColor);
  HHVM_ME(ImagickDraw, setStrokeDashArray);
  HHVM_ME(ImagickDraw, setStrokeDashOffset);
  HHVM_ME(ImagickDraw, setStrokeLineCap);
  HHVM_ME(ImagickDraw, setStrokeLineJoin);
  HHVM_ME(ImagickDraw, setStrokeMiterLimit);
  HHVM_ME(ImagickDraw, setStrokeOpacity);
  HHVM_ME(ImagickDraw, setStrokePatternURL);
  HHVM_ME(ImagickDraw, setStrokeWidth);
  HHVM_ME(ImagickDraw, setTextAlignment);
  HHVM_ME(ImagickDraw, setTextAntialias);
  HHVM_ME(ImagickDraw, setTextDecoration);
  HHVM_ME(ImagickDraw, setTextEncoding);
  HHVM_ME(ImagickDraw, setTextUnderColor);
  HHVM_ME(ImagickDraw, setVectorGraphics);
  HHVM_ME(ImagickDraw, setViewbox);
  HHVM_ME(ImagickDraw, skewX);
  HHVM_ME(ImagickDraw, skewY);
  HHVM_ME(ImagickDraw, translate);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
