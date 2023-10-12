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

  public static function example1(): ?string {
    return self::funcApply(
      function(int $x): string {
        return 'string';
      },
      10,
      vec[1, 2, 3],
    );
  }
}
