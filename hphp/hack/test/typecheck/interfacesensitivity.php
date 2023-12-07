<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface MSnackable<+T> {}

interface MFoodFlavored {}
interface MFruitFlavored extends MFoodFlavored {}

class MFood implements MSnackable<MFoodFlavored> {
  public function y(): MSnackable<MFoodFlavored> {
    return $this;
  }
}

final class MFruit extends MFood implements MSnackable<MFruitFlavored> {
    public function simple(MFruit $x): MSnackable<MFruitFlavored> {
      return $x;
    }
    <<__Override>>
    public function y(): MSnackable<MFruitFlavored> {
      return $this;
    }

    public function y2(): MSnackable<MFoodFlavored> {
      return $this;
    }

    public function y3(): MFood {
      return $this;
    }
    public static function y4(MFruit $y): MSnackable<MFruitFlavored> {
      return $y;
      }
}
