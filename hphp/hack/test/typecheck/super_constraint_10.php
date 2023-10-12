<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// From task 10355299
class MyMaps {

  /* HH_FIXME[4110] */
  public static function merge<Tk2 as arraykey, Tv2, Tk1 super Tk2 as arraykey, Tv1 super Tv2>(
    ConstMap<Tk1, Tv1> $map1,
    ConstMap<Tk2, Tv2> $map2,
  ): Map<Tk1, Tv1> {
  }

  // This should fail to check
  public static function mergeDisjoint<Tk1 as arraykey, Tv1, Tk2 super Tk1 as arraykey, Tv2 super Tv1>(
    ConstMap<Tk1, Tv1> $map1,
    ConstMap<Tk2, Tv2> $map2,
  ): Map<Tk2, Tv2> {
    $result = self::merge($map1, $map2);
    return $result;
  }
}
