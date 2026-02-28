<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class ImagickPixel {

  // Methods
  public function clear(): bool;
  public function __construct(string $color = "");
  public function destroy(): bool;
  public function getColor(bool $normalized = false): darray<string, num>;
  public function getColorAsString(): string;
  public function getColorCount(): int;
  public function getColorValue(int $color): float;
  public function getHSL(): darray<arraykey, mixed>;
  public function isPixelSimilar(
    HH\FIXME\MISSING_PARAM_TYPE $color,
    float $fuzz,
  ): bool;
  public function isSimilar(
    HH\FIXME\MISSING_PARAM_TYPE $color,
    float $fuzz,
  ): bool;
  public function setColor(string $color): bool;
  public function setColorValue(int $color, float $value): bool;
  public function setHSL(
    float $hue,
    float $saturation,
    float $luminosity,
  ): bool;

}
