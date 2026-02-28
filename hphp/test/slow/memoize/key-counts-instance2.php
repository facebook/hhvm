<?hh

class A {
  <<__Memoize>>
  public function func0A() :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo) echo "A::func0A()\n";
    return json_encode(vec[]);
  }
  <<__Memoize>>
  public function func0B() :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo) echo "A::func0B()\n";
    return json_encode(vec[]);
  }
  <<__Memoize>>
  public function func1A($p1) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func1A(".(string)$p1.")\n";
    return json_encode(vec[$p1]);
  }
  <<__Memoize>>
  public function func1B($p1) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func1B(".(string)$p1.")\n";
    return json_encode(vec[$p1]);
  }
  <<__Memoize>>
  public function func2A($p1, $p2) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func2A(".(string)$p1.", ".(string)$p2.")\n";
    return json_encode(vec[$p1, $p2]);
  }
  <<__Memoize>>
  public function func2B($p1, $p2) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func2B(".(string)$p1.", ".(string)$p2.")\n";
    return json_encode(vec[$p1, $p2]);
  }
  <<__Memoize>>
  public function func3A($p1, $p2, $p3) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func3A(".(string)$p1.", ".(string)$p2.", ".(string)$p3.")\n";
    return json_encode(vec[$p1, $p2, $p3]);
  }
  <<__Memoize>>
  public function func3B($p1, $p2, $p3) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func3B(".(string)$p1.", ".(string)$p2.", ".(string)$p3.")\n";
    return json_encode(vec[$p1, $p2, $p3]);
  }
  <<__Memoize>>
  public function func4A($p1, $p2, $p3, $p4) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func4A(".
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
  public function func4B($p1, $p2, $p3, $p4) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func4B(".
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
  public function func5A($p1, $p2, $p3, $p4, $p5) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func5A(".
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
  public function func5B($p1, $p2, $p3, $p4, $p5) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func5B(".
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
  public function func6A($p1, $p2, $p3, $p4, $p5, $p6) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func6A(".
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
  public function func6B($p1, $p2, $p3, $p4, $p5, $p6) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func6B(".
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
  public function func7A($p1, $p2, $p3, $p4, $p5, $p6, $p7) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func7A(".
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
  public function func7B($p1, $p2, $p3, $p4, $p5, $p6, $p7) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func7B(".
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
  public function func8A($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func8A(".
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
  public function func8B($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func8B(".
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
  public function func9A($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func9A(".
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
  public function func9B($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func9B(".
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
  public function func10A($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func10A(".
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
  public function func10B($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10) :mixed{

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func10B(".
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
  public function func11A(
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

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func11A(".
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
  public function func11B(
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

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func11B(".
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
  public function func12A(
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

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func12A(".
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
  <<__Memoize>>
  public function func12B(
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

    if (MemoizeKeyCountsInstance2Php::$do_echo)
      echo "A::func12B(".
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


  $x = new A();

  var_dump($x->func0A());
  var_dump($x->func1A(1.1));
  var_dump($x->func2A(1.1, 2.2));
  var_dump($x->func3A(1.1, 2.2, 3.3));
  var_dump($x->func4A(1.1, 2.2, 3.3, 4.4));
  var_dump($x->func5A(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12A(
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
      12.12,
    ),
  );
  var_dump($x->func0B());
  var_dump($x->func1B(1.1));
  var_dump($x->func2B(1.1, 2.2));
  var_dump($x->func3B(1.1, 2.2, 3.3));
  var_dump($x->func4B(1.1, 2.2, 3.3, 4.4));
  var_dump($x->func5B(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12B(
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
      12.12,
    ),
  );

  var_dump($x->func0A());
  var_dump($x->func1A(1.1));
  var_dump($x->func2A(1.1, 2.2));
  var_dump($x->func3A(1.1, 2.2, 3.3));
  var_dump($x->func4A(1.1, 2.2, 3.3, 4.4));
  var_dump($x->func5A(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12A(
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
      12.12,
    ),
  );
  var_dump($x->func0B());
  var_dump($x->func1B(1.1));
  var_dump($x->func2B(1.1, 2.2));
  var_dump($x->func3B(1.1, 2.2, 3.3));
  var_dump($x->func4B(1.1, 2.2, 3.3, 4.4));
  var_dump($x->func5B(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12B(
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
      12.12,
    ),
  );

  var_dump($x->func0A());
  var_dump($x->func1A(1.19));
  var_dump($x->func2A(1.19, 2.2));
  var_dump($x->func3A(1.19, 2.2, 3.3));
  var_dump($x->func4A(1.19, 2.2, 3.3, 4.4));
  var_dump($x->func5A(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12A(
      1.19,
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
      12.12,
    ),
  );
  var_dump($x->func0B());
  var_dump($x->func1B(1.19));
  var_dump($x->func2B(1.19, 2.2));
  var_dump($x->func3B(1.19, 2.2, 3.3));
  var_dump($x->func4B(1.19, 2.2, 3.3, 4.4));
  var_dump($x->func5B(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12B(
      1.19,
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
      12.12,
    ),
  );

  var_dump($x->func0A());
  var_dump($x->func1A(1.15));
  var_dump($x->func2A(1.1, 2.25));
  var_dump($x->func3A(1.1, 2.2, 3.35));
  var_dump($x->func4A(1.1, 2.2, 3.3, 4.45));
  var_dump($x->func5A(1.1, 2.2, 3.3, 4.4, 5.55));
  var_dump($x->func6A(1.1, 2.2, 3.3, 4.4, 5.5, 6.65));
  var_dump($x->func7A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75));
  var_dump($x->func8A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85));
  var_dump($x->func9A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95));
  var_dump($x->func10A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15));
  var_dump(
    $x->func11A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115),
  );
  var_dump(
    $x->func12A(
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
      12.125,
    ),
  );
  var_dump($x->func0B());
  var_dump($x->func1B(1.15));
  var_dump($x->func2B(1.1, 2.25));
  var_dump($x->func3B(1.1, 2.2, 3.35));
  var_dump($x->func4B(1.1, 2.2, 3.3, 4.45));
  var_dump($x->func5B(1.1, 2.2, 3.3, 4.4, 5.55));
  var_dump($x->func6B(1.1, 2.2, 3.3, 4.4, 5.5, 6.65));
  var_dump($x->func7B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75));
  var_dump($x->func8B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85));
  var_dump($x->func9B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95));
  var_dump($x->func10B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15));
  var_dump(
    $x->func11B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115),
  );
  var_dump(
    $x->func12B(
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
      12.125,
    ),
  );

  MemoizeKeyCountsInstance2Php::$do_echo = false;
  for ($i = 0; $i < 30000; $i++) {
    $x->func1A(1.15 + $i);
    $x->func2A(1.1, 2.25 + $i);
    $x->func3A(1.1, 2.2, 3.35 + $i);
    $x->func4A(1.1, 2.2, 3.3, 4.45 + $i);
    $x->func5A(1.1, 2.2, 3.3, 4.4, 5.55 + $i);
    $x->func6A(1.1, 2.2, 3.3, 4.4, 5.5, 6.65 + $i);
    $x->func7A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75 + $i);
    $x->func8A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85 + $i);
    $x->func9A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95 + $i);
    $x->func10A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15 + $i);
    $x->func11A(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115 + $i);
    $x->func12A(
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
    $x->func1B(1.15 + $i);
    $x->func2B(1.1, 2.25 + $i);
    $x->func3B(1.1, 2.2, 3.35 + $i);
    $x->func4B(1.1, 2.2, 3.3, 4.45 + $i);
    $x->func5B(1.1, 2.2, 3.3, 4.4, 5.55 + $i);
    $x->func6B(1.1, 2.2, 3.3, 4.4, 5.5, 6.65 + $i);
    $x->func7B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75 + $i);
    $x->func8B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85 + $i);
    $x->func9B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95 + $i);
    $x->func10B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15 + $i);
    $x->func11B(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115 + $i);
    $x->func12B(
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

  MemoizeKeyCountsInstance2Php::$do_echo = true;
  var_dump($x->func0A());
  var_dump($x->func1A(1.19));
  var_dump($x->func2A(1.19, 2.2));
  var_dump($x->func3A(1.19, 2.2, 3.3));
  var_dump($x->func4A(1.19, 2.2, 3.3, 4.4));
  var_dump($x->func5A(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11A(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12A(
      1.19,
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
      12.12,
    ),
  );
  var_dump($x->func0B());
  var_dump($x->func1B(1.19));
  var_dump($x->func2B(1.19, 2.2));
  var_dump($x->func3B(1.19, 2.2, 3.3));
  var_dump($x->func4B(1.19, 2.2, 3.3, 4.4));
  var_dump($x->func5B(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump($x->func6B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump($x->func7B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump($x->func8B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump($x->func9B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump($x->func10B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(
    $x->func11B(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11),
  );
  var_dump(
    $x->func12B(
      1.19,
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
      12.12,
    ),
  );
}

abstract final class MemoizeKeyCountsInstance2Php {
  public static $do_echo;
}
<<__EntryPoint>>
function entrypoint_keycountsinstance2(): void {
  // Copyright 2004-present Facebook. All Rights Reserved.

  MemoizeKeyCountsInstance2Php::$do_echo = true;
  test();
}
