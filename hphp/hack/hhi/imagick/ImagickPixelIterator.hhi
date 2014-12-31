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

class ImagickPixelIterator implements
  KeyedTraversable<int, array<int, ImagickPixel>>,
  Iterator<array<int, ImagickPixel>> {

  // Methods
  public static function getPixelIterator(
    Imagick $wand,
  ): ImagickPixelIterator;
  public static function getPixelRegionIterator(
    Imagick $wand,
    int $x,
    int $y,
    int $columns,
    int $rows,
  ): ImagickPixelIterator;
  public function current(): array<int, ImagickPixel>;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
  public function clear(): bool;
  public function __construct(Imagick $wand);
  public function destroy(): bool;
  public function getCurrentIteratorRow(): array<int, ImagickPixel>;
  public function getIteratorRow(): int;
  public function getNextIteratorRow(): array<int, ImagickPixel>;
  public function getPreviousIteratorRow(): array<int, ImagickPixel>;
  public function newPixelIterator(Imagick $wand): bool;
  public function newPixelRegionIterator(
    Imagick $wand,
    int $x,
    int $y,
    int $columns,
    int $rows,
  ): bool;
  public function resetIterator(): bool;
  public function setIteratorFirstRow(): bool;
  public function setIteratorLastRow(): bool;
  public function setIteratorRow(int $row): bool;
  public function syncIterator(): bool;

}
