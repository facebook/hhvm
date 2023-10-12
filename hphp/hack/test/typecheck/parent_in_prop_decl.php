<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  const A = 5;
  public static int $x = parent::class;
  public static mixed $y = vec[parent::class];
}
