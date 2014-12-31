<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class ImagickDraw {

  // Methods
  public function affine(array $affine): bool;
  public function annotation(float $x, float $y, string $text): bool;
  public function arc(
    float $sx,
    float $sy,
    float $ex,
    float $ey,
    float $sd,
    float $ed,
  ): bool;
  public function bezier(array $coordinates): bool;
  public function circle(float $ox, float $oy, float $px, float $py): bool;
  public function clear(): bool;
  public function __clone(): void;
  public function color(float $x, float $y, int $paintMethod): bool;
  public function comment(string $comment): bool;
  public function composite(
    int $compose,
    float $x,
    float $y,
    float $width,
    float $height,
    Imagick $compositeWand,
  ): bool;
  public function __construct();
  public function destroy(): bool;
  public function ellipse(
    float $ox,
    float $oy,
    float $rx,
    float $ry,
    float $start,
    float $end,
  ): bool;
  public function getClipPath(): string;
  public function getClipRule(): int;
  public function getClipUnits(): int;
  public function getFillColor(): ImagickPixel;
  public function getFillOpacity(): float;
  public function getFillRule(): int;
  public function getFont(): string;
  public function getFontFamily(): string;
  public function getFontSize(): float;
  public function getFontStretch(): int;
  public function getFontStyle(): int;
  public function getFontWeight(): int;
  public function getGravity(): int;
  public function getStrokeAntialias(): bool;
  public function getStrokeColor(): ImagickPixel;
  public function getStrokeDashArray(): array;
  public function getStrokeDashOffset(): float;
  public function getStrokeLineCap(): int;
  public function getStrokeLineJoin(): int;
  public function getStrokeMiterLimit(): int;
  public function getStrokeOpacity(): float;
  public function getStrokeWidth(): float;
  public function getTextAlignment(): int;
  public function getTextAntialias(): bool;
  public function getTextDecoration(): int;
  public function getTextEncoding(): string;
  public function getTextUnderColor(): ImagickPixel;
  public function getVectorGraphics(): string;
  public function line(float $sx, float $sy, float $ex, float $ey): bool;
  public function matte(float $x, float $y, int $paintMethod): bool;
  public function pathClose(): bool;
  public function pathCurveToAbsolute(
    float $x1,
    float $y1,
    float $x2,
    float $y2,
    float $x,
    float $y,
  ): bool;
  public function pathCurveToQuadraticBezierAbsolute(
    float $x1,
    float $y1,
    float $x,
    float $y,
  ): bool;
  public function pathCurveToQuadraticBezierRelative(
    float $x1,
    float $y1,
    float $x,
    float $y,
  ): bool;
  public function pathCurveToQuadraticBezierSmoothAbsolute(
    float $x,
    float $y,
  ): bool;
  public function pathCurveToQuadraticBezierSmoothRelative(
    float $x,
    float $y,
  ): bool;
  public function pathCurveToRelative(
    float $x1,
    float $y1,
    float $x2,
    float $y2,
    float $x,
    float $y,
  ): bool;
  public function pathCurveToSmoothAbsolute(
    float $x2,
    float $y2,
    float $x,
    float $y,
  ): bool;
  public function pathCurveToSmoothRelative(
    float $x2,
    float $y2,
    float $x,
    float $y,
  ): bool;
  public function pathEllipticArcAbsolute(
    float $rx,
    float $ry,
    float $x_axis_rotation,
    bool $large_arc_flag,
    bool $sweep_flag,
    float $x,
    float $y,
  ): bool;
  public function pathEllipticArcRelative(
    float $rx,
    float $ry,
    float $x_axis_rotation,
    bool $large_arc_flag,
    bool $sweep_flag,
    float $x,
    float $y,
  ): bool;
  public function pathFinish(): bool;
  public function pathLineToAbsolute(float $x, float $y): bool;
  public function pathLineToHorizontalAbsolute(float $x): bool;
  public function pathLineToHorizontalRelative(float $x): bool;
  public function pathLineToRelative(float $x, float $y): bool;
  public function pathLineToVerticalAbsolute(float $y): bool;
  public function pathLineToVerticalRelative(float $y): bool;
  public function pathMoveToAbsolute(float $x, float $y): bool;
  public function pathMoveToRelative(float $x, float $y): bool;
  public function pathStart(): bool;
  public function point(float $x, float $y): bool;
  public function polygon(array $coordinates): bool;
  public function polyline(array $coordinates): bool;
  public function pop(): bool;
  public function popClipPath(): bool;
  public function popDefs(): bool;
  public function popPattern(): bool;
  public function push(): bool;
  public function pushClipPath(string $clip_mask_id): bool;
  public function pushDefs(): bool;
  public function pushPattern(
    string $pattern_id,
    float $x,
    float $y,
    float $width,
    float $height,
  ): bool;
  public function rectangle(float $x1, float $y1, float $x2, float $y2): bool;
  public function render(): bool;
  public function rotate(float $degrees): bool;
  public function roundRectangle(
    float $x1,
    float $y1,
    float $x2,
    float $y2,
    float $rx,
    float $ry,
  ): bool;
  public function scale(float $x, float $y): bool;
  public function setClipPath(string $clip_mask): bool;
  public function setClipRule(int $fill_rule): bool;
  public function setClipUnits(int $clip_units): bool;
  public function setFillAlpha(float $opacity): bool;
  public function setFillColor($fill_pixel): bool;
  public function setFillOpacity(float $fillOpacity): bool;
  public function setFillPatternURL(string $fill_url): bool;
  public function setFillRule(int $fill_rule): bool;
  public function setFont(string $font_name): bool;
  public function setFontFamily(string $font_family): bool;
  public function setFontSize(float $pointsize): bool;
  public function setFontStretch(int $fontStretch): bool;
  public function setFontStyle(int $style): bool;
  public function setFontWeight(int $font_weight): bool;
  public function setGravity(int $gravity): bool;
  public function setResolution(float $x, float $y): bool;
  public function setStrokeAlpha(float $opacity): bool;
  public function setStrokeAntialias(bool $stroke_antialias): bool;
  public function setStrokeColor($stroke_pixel): bool;
  public function setStrokeDashArray(array $dashArray): bool;
  public function setStrokeDashOffset(float $dash_offset): bool;
  public function setStrokeLineCap(int $linecap): bool;
  public function setStrokeLineJoin(int $linejoin): bool;
  public function setStrokeMiterLimit(int $miterlimit): bool;
  public function setStrokeOpacity(float $stroke_opacity): bool;
  public function setStrokePatternURL(string $stroke_url): bool;
  public function setStrokeWidth(float $stroke_width): bool;
  public function setTextAlignment(int $alignment): bool;
  public function setTextAntialias(bool $antiAlias): bool;
  public function setTextDecoration(int $decoration): bool;
  public function setTextEncoding(string $encoding): bool;
  public function setTextUnderColor($under_color): bool;
  public function setVectorGraphics(string $xml): bool;
  public function setViewbox(int $x1, int $y1, int $x2, int $y2): bool;
  public function skewX(float $degrees): bool;
  public function skewY(float $degrees): bool;
  public function translate(float $x, float $y): bool;

}
