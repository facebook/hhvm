<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class ImagickPixelIterator implements
  KeyedTraversable<int, varray<ImagickPixel>>,
  Iterator<varray<ImagickPixel>> {

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
  public function current(): varray<ImagickPixel>;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
  public function clear(): bool;
  public function __construct(Imagick $wand);
  public function destroy(): bool;
  public function getCurrentIteratorRow(): varray<ImagickPixel>;
  public function getIteratorRow(): int;
  public function getNextIteratorRow(): varray<ImagickPixel>;
  public function getPreviousIteratorRow(): varray<ImagickPixel>;
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
