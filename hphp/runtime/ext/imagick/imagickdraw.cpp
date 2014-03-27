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
ALWAYS_INLINE
static void getAffineMatrixElement(
    const Array& array, const String& key, double& ret) {
  const Variant& value = array.rvalAtRef(key);
  if (value.isNull()) {
    IMAGICKDRAW_THROW(
      "AffineMatrix must contain keys: sx, rx, ry, sy, tx and ty");
  } else {
    ret = value.toDouble();
  }
}

static bool HHVM_METHOD(ImagickDraw, affine, const Array& affine) {
  auto wand = getDrawingWandResource(this_);
  AffineMatrix affineMatrix;
  getAffineMatrixElement(affine, s_sx, affineMatrix.sx);
  getAffineMatrixElement(affine, s_rx, affineMatrix.rx);
  getAffineMatrixElement(affine, s_ry, affineMatrix.ry);
  getAffineMatrixElement(affine, s_sy, affineMatrix.sy);
  getAffineMatrixElement(affine, s_tx, affineMatrix.tx);
  getAffineMatrixElement(affine, s_ty, affineMatrix.ty);
  DrawAffine(wand->getWand(), &affineMatrix);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, annotation,
    double x, double y, const String& text) {
  auto wand = getDrawingWandResource(this_);
  DrawAnnotation(wand->getWand(), x, y, (CUCString)text.c_str());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, arc, double sx, double sy,
                                          double ex, double ey,
                                          double sd, double ed) {
  auto wand = getDrawingWandResource(this_);
  DrawArc(wand->getWand(), sx, sy, ex, ey, sd, ed);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, bezier,
    const Array& coordinates) {
  auto wand = getDrawingWandResource(this_);
  auto points = toPointInfoArray(coordinates);
  if (points.empty()) {
    IMAGICKDRAW_THROW("Unable to read coordinate array");
  }
  DrawBezier(wand->getWand(), points.size(), points.data());
  return true;
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

static void HHVM_METHOD(ImagickDraw, __clone) {
  auto wand = getDrawingWandResource(this_);
  auto newWand = CloneDrawingWand(wand->getWand());
  if (newWand == nullptr) {
    IMAGICKDRAW_THROW("Failed to allocate DrawingWand structure");
  } else {
    setWandResource(s_ImagickDraw, this_, newWand);
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
                        int64_t compose,
                        double x, double y, double width, double height,
                        const Object compositeWand) {
  auto wand = getDrawingWandResource(this_);
  Object compositeWandObj(compositeWand);
  auto magick = getMagickWandResource(compositeWandObj);
  auto status = DrawComposite(wand->getWand(), (CompositeOperator)compose,
                              x, y, width, height, magick->getWand());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Compositing image failed");
  }
  return true;
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
                        double ox, double oy, double rx, double ry,
                        double start, double end) {
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
  return DrawGetClipRule(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getClipUnits) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetClipUnits(wand->getWand());
}

static Object HHVM_METHOD(ImagickDraw, getFillColor) {
  auto wand = getDrawingWandResource(this_);
  auto pixel = NewPixelWand();
  DrawGetFillColor(wand->getWand(), pixel);
  return createImagickPixel(pixel, true);
}

static double HHVM_METHOD(ImagickDraw, getFillOpacity) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFillOpacity(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getFillRule) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFillRule(wand->getWand());
}

static String HHVM_METHOD(ImagickDraw, getFont) {
  auto wand = getDrawingWandResource(this_);
  return convertMagickString(DrawGetFont(wand->getWand()));
}

static String HHVM_METHOD(ImagickDraw, getFontFamily) {
  auto wand = getDrawingWandResource(this_);
  return convertMagickString(DrawGetFontFamily(wand->getWand()));
}

static double HHVM_METHOD(ImagickDraw, getFontSize) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFontSize(wand->getWand());
}

static int HHVM_METHOD(ImagickDraw, getFontStretch) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFontStretch(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getFontStyle) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFontStyle(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getFontWeight) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetFontWeight(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getGravity) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetGravity(wand->getWand());
}

static bool HHVM_METHOD(ImagickDraw, getStrokeAntialias) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeAntialias(wand->getWand()) != MagickFalse;
}

static Object HHVM_METHOD(ImagickDraw, getStrokeColor) {
  auto wand = getDrawingWandResource(this_);
  auto pixel = NewPixelWand();
  DrawGetStrokeColor(wand->getWand(), pixel);
  return createImagickPixel(pixel, true);
}

static Array HHVM_METHOD(ImagickDraw, getStrokeDashArray) {
  auto wand = getDrawingWandResource(this_);
  size_t num;
  double* arr = DrawGetStrokeDashArray(wand->getWand(), &num);
  return convertMagickArray(num, arr);
}

static double HHVM_METHOD(ImagickDraw, getStrokeDashOffset) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeDashOffset(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeLineCap) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeLineCap(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeLineJoin) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeLineJoin(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getStrokeMiterLimit) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeMiterLimit(wand->getWand());
}

static double HHVM_METHOD(ImagickDraw, getStrokeOpacity) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeOpacity(wand->getWand());
}

static double HHVM_METHOD(ImagickDraw, getStrokeWidth) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetStrokeWidth(wand->getWand());
}

static int64_t HHVM_METHOD(ImagickDraw, getTextAlignment) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetTextAlignment(wand->getWand());
}

static bool HHVM_METHOD(ImagickDraw, getTextAntialias) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetTextAntialias(wand->getWand()) != MagickFalse;
}

static int64_t HHVM_METHOD(ImagickDraw, getTextDecoration) {
  auto wand = getDrawingWandResource(this_);
  return DrawGetTextDecoration(wand->getWand());
}

static String HHVM_METHOD(ImagickDraw, getTextEncoding) {
  auto wand = getDrawingWandResource(this_);
  return convertMagickString(DrawGetTextEncoding(wand->getWand()));
}

static Object HHVM_METHOD(ImagickDraw, getTextUnderColor) {
  auto wand = getDrawingWandResource(this_);
  auto pixel = NewPixelWand();
  DrawGetTextUnderColor(wand->getWand(), pixel);
  return createImagickPixel(pixel, true);
}

static String HHVM_METHOD(ImagickDraw, getVectorGraphics) {
  auto wand = getDrawingWandResource(this_);
  return convertMagickString(DrawGetVectorGraphics(wand->getWand()));
}

static bool HHVM_METHOD(ImagickDraw, line,
    double sx, double sy, double ex, double ey) {
  auto wand = getDrawingWandResource(this_);
  DrawLine(wand->getWand(), sx, sy, ex, ey);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, matte,
    double x, double y, int64_t paintMethod) {
  auto wand = getDrawingWandResource(this_);
  DrawMatte(wand->getWand(), x, y, (PaintMethod)paintMethod);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathClose) {
  auto wand = getDrawingWandResource(this_);
  DrawPathClose(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToAbsolute,
                        double x1, double y1, double x2, double y2,
                        double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToAbsolute(wand->getWand(), x1, y1, x2, y2, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierAbsolute,
    double x1, double y1, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToQuadraticBezierAbsolute(wand->getWand(), x1, y1, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierRelative,
    double x1, double y1, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToQuadraticBezierRelative(wand->getWand(), x1, y1, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierSmoothAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToQuadraticBezierSmoothAbsolute(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToQuadraticBezierSmoothRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToQuadraticBezierSmoothRelative(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToRelative,
                        double x1, double y1, double x2, double y2,
                        double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToRelative(wand->getWand(), x1, y1, x2, y2, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToSmoothAbsolute,
    double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToSmoothAbsolute(wand->getWand(), x2, y2, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathCurveToSmoothRelative,
    double x2, double y2, double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathCurveToSmoothRelative(wand->getWand(), x2, y2, x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathEllipticArcAbsolute,
                        double rx, double ry, double x_axis_rotation,
                        bool large_arc_flag, bool sweep_flag,
                        double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathEllipticArcAbsolute(wand->getWand(), rx, ry, x_axis_rotation,
    toMagickBool(large_arc_flag), toMagickBool(sweep_flag), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathEllipticArcRelative,
                        double rx, double ry, double x_axis_rotation,
                        bool large_arc_flag, bool sweep_flag,
                        double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathEllipticArcRelative(wand->getWand(), rx, ry, x_axis_rotation,
    toMagickBool(large_arc_flag), toMagickBool(sweep_flag), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathFinish) {
  auto wand = getDrawingWandResource(this_);
  DrawPathFinish(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToAbsolute(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToHorizontalAbsolute, double x) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToHorizontalAbsolute(wand->getWand(), x);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToHorizontalRelative, double x) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToHorizontalRelative(wand->getWand(), x);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToRelative(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToVerticalAbsolute, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToVerticalAbsolute(wand->getWand(), y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathLineToVerticalRelative, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathLineToVerticalRelative(wand->getWand(), y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathMoveToAbsolute,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathMoveToAbsolute(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathMoveToRelative,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPathMoveToRelative(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pathStart) {
  auto wand = getDrawingWandResource(this_);
  DrawPathStart(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, point,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawPoint(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, polygon,
    const Array& coordinates) {
  auto wand = getDrawingWandResource(this_);
  auto points = toPointInfoArray(coordinates);
  if (points.empty()) {
    IMAGICKDRAW_THROW("Unable to read coordinate array");
  }
  DrawPolygon(wand->getWand(), points.size(), points.data());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, polyline,
    const Array& coordinates) {
  auto wand = getDrawingWandResource(this_);
  auto points = toPointInfoArray(coordinates);
  if (points.empty()) {
    IMAGICKDRAW_THROW("Unable to read coordinate array");
  }
  DrawPolyline(wand->getWand(), points.size(), points.data());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pop) {
  auto wand = getDrawingWandResource(this_);
  auto status = PopDrawingWand(wand->getWand());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to pop the current ImagickDraw object");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, popClipPath) {
  auto wand = getDrawingWandResource(this_);
  DrawPopClipPath(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, popDefs) {
  auto wand = getDrawingWandResource(this_);
  DrawPopDefs(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, popPattern) {
  auto wand = getDrawingWandResource(this_);
  auto status = DrawPopPattern(wand->getWand());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to terminate the pattern definition");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, push) {
  auto wand = getDrawingWandResource(this_);
  auto status = PushDrawingWand(wand->getWand());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to push the current ImagickDraw object");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pushClipPath,
    const String& clip_mask_id) {
  auto wand = getDrawingWandResource(this_);
  DrawPushClipPath(wand->getWand(), clip_mask_id.c_str());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pushDefs) {
  auto wand = getDrawingWandResource(this_);
  DrawPushDefs(wand->getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, pushPattern,
    const String& pattern_id, double x, double y, double width, double height) {
  auto wand = getDrawingWandResource(this_);
  DrawPushPattern(wand->getWand(), pattern_id.c_str(), x, y, width, height);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, rectangle,
    double x1, double y1, double x2, double y2) {
  auto wand = getDrawingWandResource(this_);
  DrawRectangle(wand->getWand(), x1, y1, x2, y2);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, render) {
  auto wand = getDrawingWandResource(this_);
  auto status = withMagickLocaleFix([&wand]{
    return DrawRender(wand->getWand());
  });
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to render the drawing commands");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, rotate, double degrees) {
  auto wand = getDrawingWandResource(this_);
  DrawRotate(wand->getWand(), degrees);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, roundRectangle,
                        double x1, double y1, double x2, double y2,
                        double rx, double ry) {
  auto wand = getDrawingWandResource(this_);
  DrawRoundRectangle(wand->getWand(), x1, y1, x2, y2, rx, ry);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, scale,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawScale(wand->getWand(), x, y);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setClipPath,
    const String& clip_mask) {
  auto wand = getDrawingWandResource(this_);
  auto status = DrawSetClipPath(wand->getWand(), clip_mask.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set clipping path");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setClipRule,
    int64_t fill_rule) {
  auto wand = getDrawingWandResource(this_);
  DrawSetClipRule(wand->getWand(), (FillRule)fill_rule);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setClipUnits,
    int64_t clip_units) {
  auto wand = getDrawingWandResource(this_);
  DrawSetClipUnits(wand->getWand(), (ClipPathUnits)clip_units);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFillAlpha, double opacity) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFillOpacity(wand->getWand(), opacity);
  throw true;
}

static bool HHVM_METHOD(ImagickDraw, setFillColor,
    const Variant& fill_pixel) {
  auto wand = getDrawingWandResource(this_);
  WandResource<PixelWand> pixel(buildColorWand(fill_pixel));
  DrawSetFillColor(wand->getWand(), pixel.getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFillOpacity,
    double fillOpacity) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFillOpacity(wand->getWand(), fillOpacity);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFillPatternURL,
    const String& fill_url) {
  auto wand = getDrawingWandResource(this_);
  auto status = DrawSetFillPatternURL(wand->getWand(), fill_url.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set fill pattern URL");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFillRule,
    int64_t fill_rule) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFillRule(wand->getWand(), (FillRule)fill_rule);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFont,
    const String& font_name) {
  auto wand = getDrawingWandResource(this_);
  if (font_name.empty()) {
    IMAGICKDRAW_THROW("Can not set empty font");
  }
  auto font = magickResolveFont(font_name);
  if (font.isNull()) {
    IMAGICKDRAW_THROW("Unable to set font, file path expansion failed");
  }
  auto status = DrawSetFont(wand->getWand(), font.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set font");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFontFamily,
    const String& font_family) {
  auto wand = getDrawingWandResource(this_);
  if (font_family.empty()) {
    IMAGICKDRAW_THROW("Can not set empty font family");
  }
  auto status = DrawSetFontFamily(wand->getWand(), font_family.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set font family");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFontSize, double pointsize) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFontSize(wand->getWand(), pointsize);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFontStretch,
    int64_t fontStretch) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFontStretch(wand->getWand(), (StretchType)fontStretch);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFontStyle, int64_t style) {
  auto wand = getDrawingWandResource(this_);
  DrawSetFontStyle(wand->getWand(), (StyleType)style);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setFontWeight,
    int64_t font_weight) {
  auto wand = getDrawingWandResource(this_);
  if (100 <= font_weight && font_weight <= 900) {
    DrawSetFontWeight(wand->getWand(), font_weight);
    return true;
  } else {
    IMAGICKDRAW_THROW("Font weight valid range is 100-900");
  }
}

static bool HHVM_METHOD(ImagickDraw, setGravity, int64_t gravity) {
  auto wand = getDrawingWandResource(this_);
  DrawSetGravity(wand->getWand(), (GravityType)gravity);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setResolution,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  std::ostringstream density;
  density << x << "x" << y;

  auto drawInfo = PeekDrawingWand(wand->getWand());
  drawInfo->density = AcquireString(density.str().c_str());
  auto drawWand = DrawAllocateWand(drawInfo, nullptr);
  if (drawWand == nullptr) {
    IMAGICKDRAW_THROW("Failed to allocate new DrawingWand structure");
  }
  setWandResource(s_ImagickDraw, this_, drawWand);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeAlpha, double opacity) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeOpacity(wand->getWand(), opacity);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeAntialias,
    bool stroke_antialias) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeAntialias(wand->getWand(), toMagickBool(stroke_antialias));
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeColor,
    const Variant& stroke_pixel) {
  auto wand = getDrawingWandResource(this_);
  WandResource<PixelWand> pixel(buildColorWand(stroke_pixel));
  DrawSetStrokeColor(wand->getWand(), pixel.getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeDashArray,
    const Array& dashArray) {
  auto wand = getDrawingWandResource(this_);
  auto dashes = toDoubleArray(dashArray);
  if (dashes.empty()) {
    IMAGICKDRAW_THROW("Cannot read stroke dash array parameter");
  }
  DrawSetStrokeDashArray(wand->getWand(), dashes.size(), dashes.data());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeDashOffset,
    double dash_offset) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeDashOffset(wand->getWand(), dash_offset);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeLineCap, int64_t linecap) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeLineCap(wand->getWand(), (LineCap)linecap);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeLineJoin, int64_t linejoin) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeLineJoin(wand->getWand(), (LineJoin)linejoin);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeMiterLimit,
    int64_t miterlimit) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeMiterLimit(wand->getWand(), miterlimit);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeOpacity,
    double stroke_opacity) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeOpacity(wand->getWand(), stroke_opacity);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokePatternURL,
    const String& stroke_url) {
  auto wand = getDrawingWandResource(this_);
  auto status = DrawSetStrokePatternURL(wand->getWand(), stroke_url.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set stroke pattern URL");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setStrokeWidth,
    double stroke_width) {
  auto wand = getDrawingWandResource(this_);
  DrawSetStrokeWidth(wand->getWand(), stroke_width);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setTextAlignment,
    int64_t alignment) {
  auto wand = getDrawingWandResource(this_);
  DrawSetTextAlignment(wand->getWand(), (AlignType)alignment);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setTextAntialias, bool antiAlias) {
  auto wand = getDrawingWandResource(this_);
  DrawSetTextAntialias(wand->getWand(), toMagickBool(antiAlias));
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setTextDecoration,
    int64_t decoration) {
  auto wand = getDrawingWandResource(this_);
  DrawSetTextDecoration(wand->getWand(), (DecorationType)decoration);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setTextEncoding,
    const String& encoding) {
  auto wand = getDrawingWandResource(this_);
  DrawSetTextEncoding(wand->getWand(), encoding.c_str());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setTextUnderColor,
    const Variant& under_color) {
  auto wand = getDrawingWandResource(this_);
  WandResource<PixelWand> pixel(buildColorWand(under_color));
  DrawSetTextUnderColor(wand->getWand(), pixel.getWand());
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setVectorGraphics,
    const String& xml) {
  auto wand = getDrawingWandResource(this_);
  auto status = DrawSetVectorGraphics(wand->getWand(), xml.c_str());
  if (status == MagickFalse) {
    IMAGICKDRAW_THROW("Unable to set the vector graphics");
  }
  return true;
}

static bool HHVM_METHOD(ImagickDraw, setViewbox,
    int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
  auto wand = getDrawingWandResource(this_);
  DrawSetViewbox(wand->getWand(), x1, y1, x2, y2);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, skewX, double degrees) {
  auto wand = getDrawingWandResource(this_);
  DrawSkewX(wand->getWand(), degrees);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, skewY, double degrees) {
  auto wand = getDrawingWandResource(this_);
  DrawSkewY(wand->getWand(), degrees);
  return true;
}

static bool HHVM_METHOD(ImagickDraw, translate,
    double x, double y) {
  auto wand = getDrawingWandResource(this_);
  DrawTranslate(wand->getWand(), x, y);
  return true;
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
  HHVM_ME(ImagickDraw, getFontStretch);
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
  HHVM_ME(ImagickDraw, setResolution);
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
