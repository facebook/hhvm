<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C<Tk as arraykey, Tv>
{

  private dict<Tk, Tv> $d = dict[];

  public function __construct() {
  }

  public function set2<Tk2 as arraykey, Tv_>(
    Tk $key1,
    Tk2 $key2,
    Tv_ $value,
  ): void where Tv = dict<Tk2, Tv_> {
    $this->d[$key1] ??= dict[];
    $this->d[$key1][$key2] = $value;
  }
}
