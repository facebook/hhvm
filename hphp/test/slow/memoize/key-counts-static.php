<?hh

class A {
  <<__Memoize>>
  public static function func0() :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo) echo "A::func0()\n";
    return json_encode(vec[]);
  }
  <<__Memoize>>
  public static function func1($p1) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo) echo "A::func1(".(string)$p1.")\n";
    return json_encode(vec[$p1]);
  }
  <<__Memoize>>
  public static function func2($p1, $p2) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func2(".(string)$p1.", ".(string)$p2.")\n";
    return json_encode(vec[$p1, $p2]);
  }
  <<__Memoize>>
  public static function func3($p1, $p2, $p3) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func3(".(string)$p1.", ".(string)$p2.", ".(string)$p3.")\n";
    return json_encode(vec[$p1, $p2, $p3]);
  }
  <<__Memoize>>
  public static function func4($p1, $p2, $p3, $p4) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func4(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4]);
  }
  <<__Memoize>>
  public static function func5($p1, $p2, $p3, $p4, $p5) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func5(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4, $p5]);
  }
  <<__Memoize>>
  public static function func6($p1, $p2, $p3, $p4, $p5, $p6) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func6(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4, $p5, $p6]);
  }
  <<__Memoize>>
  public static function func7($p1, $p2, $p3, $p4, $p5, $p6, $p7) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func7(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7]);
  }
  <<__Memoize>>
  public static function func8($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func8(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ", ".
        (string)$p8.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8]);
  }
  <<__Memoize>>
  public static function func9($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func9(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ", ".
        (string)$p8.
        ", ".
        (string)$p9.
        ")\n";
    return json_encode(vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9]);
  }
  <<__Memoize>>
  public static function func10(
    $p1,
    $p2,
    $p3,
    $p4,
    $p5,
    $p6,
    $p7,
    $p8,
    $p9,
    $p10,
  ) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func10(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ", ".
        (string)$p8.
        ", ".
        (string)$p9.
        ", ".
        (string)$p10.
        ")\n";
    return json_encode(
      vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10],
    );
  }
  <<__Memoize>>
  public static function func11(
    $p1,
    $p2,
    $p3,
    $p4,
    $p5,
    $p6,
    $p7,
    $p8,
    $p9,
    $p10,
    $p11,
  ) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func11(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ", ".
        (string)$p8.
        ", ".
        (string)$p9.
        ", ".
        (string)$p10.
        ", ".
        (string)$p11.
        ")\n";
    return json_encode(
      vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11],
    );
  }
  <<__Memoize>>
  public static function func12(
    $p1,
    $p2,
    $p3,
    $p4,
    $p5,
    $p6,
    $p7,
    $p8,
    $p9,
    $p10,
    $p11,
    $p12,
  ) :mixed{

    if (MemoizeKeyCountsStaticPhp::$do_echo)
      echo "A::func12(".
        (string)$p1.
        ", ".
        (string)$p2.
        ", ".
        (string)$p3.
        ", ".
        (string)$p4.
        ", ".
        (string)$p5.
        ", ".
        (string)$p6.
        ", ".
        (string)$p7.
        ", ".
        (string)$p8.
        ", ".
        (string)$p9.
        ", ".
        (string)$p10.
        ", ".
        (string)$p11.
        ", ".
        (string)$p12.
        ")\n";
    return json_encode(
      vec[$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11, $p12],
    );
  }
}

function test() :mixed{


  var_dump(A::func0());
  var_dump(A::func1(1.1));
  var_dump(A::func2(1.1, 2.2));
  var_dump(A::func3(1.1, 2.2, 3.3));
  var_dump(A::func4(1.1, 2.2, 3.3, 4.4));
  var_dump(A::func5(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump(A::func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(A::func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(A::func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(A::func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(A::func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(A::func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(
    A::func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12),
  );

  var_dump(A::func0());
  var_dump(A::func1(1.1));
  var_dump(A::func2(1.1, 2.2));
  var_dump(A::func3(1.1, 2.2, 3.3));
  var_dump(A::func4(1.1, 2.2, 3.3, 4.4));
  var_dump(A::func5(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump(A::func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(A::func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(A::func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(A::func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(A::func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(A::func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(
    A::func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12),
  );

  var_dump(A::func0());
  var_dump(A::func1(1.19));
  var_dump(A::func2(1.19, 2.2));
  var_dump(A::func3(1.19, 2.2, 3.3));
  var_dump(A::func4(1.19, 2.2, 3.3, 4.4));
  var_dump(A::func5(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump(A::func6(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(A::func7(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(A::func8(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(A::func9(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(A::func10(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    A::func11(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    A::func12(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12),
  );

  var_dump(A::func0());
  var_dump(A::func1(1.15));
  var_dump(A::func2(1.1, 2.25));
  var_dump(A::func3(1.1, 2.2, 3.35));
  var_dump(A::func4(1.1, 2.2, 3.3, 4.45));
  var_dump(A::func5(1.1, 2.2, 3.3, 4.4, 5.55));
  var_dump(A::func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.65));
  var_dump(A::func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75));
  var_dump(A::func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85));
  var_dump(A::func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95));
  var_dump(A::func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15));
  var_dump(
    A::func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115),
  );
  var_dump(
    A::func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.125),
  );

  MemoizeKeyCountsStaticPhp::$do_echo = false;
  for ($i = 0; $i < 30000; $i++) {
    A::func1(1.15 + $i);
    A::func2(1.1, 2.25 + $i);
    A::func3(1.1, 2.2, 3.35 + $i);
    A::func4(1.1, 2.2, 3.3, 4.45 + $i);
    A::func5(1.1, 2.2, 3.3, 4.4, 5.55 + $i);
    A::func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.65 + $i);
    A::func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75 + $i);
    A::func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85 + $i);
    A::func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95 + $i);
    A::func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15 + $i);
    A::func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115 + $i);
    A::func12(
      1.1,
      2.2,
      3.3,
      4.4,
      5.5,
      6.6,
      7.7,
      8.8,
      9.9,
      10.1,
      11.11,
      12.125 + $i,
    );
  }

  MemoizeKeyCountsStaticPhp::$do_echo = true;
  var_dump(A::func0());
  var_dump(A::func1(1.19));
  var_dump(A::func2(1.19, 2.2));
  var_dump(A::func3(1.19, 2.2, 3.3));
  var_dump(A::func4(1.19, 2.2, 3.3, 4.4));
  var_dump(A::func5(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump(A::func6(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(A::func7(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(A::func8(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(A::func9(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(A::func10(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    A::func11(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    A::func12(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12),
  );

}

abstract final class MemoizeKeyCountsStaticPhp {
  public static $do_echo;
}
<<__EntryPoint>>
function entrypoint_keycountsstatic(): void {
  // Copyright 2004-present Facebook. All Rights Reserved.

  MemoizeKeyCountsStaticPhp::$do_echo = true;
  test();
}
