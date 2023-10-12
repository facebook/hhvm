<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static function funcApply<Treturn, T1, T2>(
    (function(T1, T2): ?Treturn) $func,
    T1 $input1,
    T2 $input2,
  ): ?Treturn {
    return $func($input1, $input2);
  }

  public static function example2(): ?int {
    return self::funcApply(
      function(int $x, int $y, string $z): int {
        return $x + $y;
      },
      1,
      2,
    );
  }
}
