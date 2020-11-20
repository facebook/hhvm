<?hh
// Copyright 2004-present Facebook. All Rights Reserved.



<<__Memoize>> function func1($x) {}
<<__Memoize, __DynamicallyCallable>> function func4($x) {}

class A {
  <<__Memoize>> public function func2($x) {}
  <<__Memoize>> public static function func3($x) {}

  public static function positive_tests() {
    echo "====================== positive tests (A) ==================\n";
    $v = 123;

    $x = 'func1'; $x($v);
    $x = 'func1$memoize_impl'; $x($v);
    $x = 'func4$memoize_impl'; $x($v);
    //$x = 'A::func2'; $x($v); // fatal
    //$x = 'A::func2$memoize_impl'; $x($v); // fatal
    $x = 'A::func3'; $x($v);
    $x = 'A::func3$memoize_impl'; $x($v);

    //$x = varray['A', 'func2']; $x($v); // fatal
    //$x = varray['A', 'func2$memoize_impl']; $x($v); // fatal
    $x = varray['A', 'func3']; $x($v);
    $x = varray['A', 'func3$memoize_impl']; $x($v);

    $x = varray[new A, 'func2']; $x($v);
    $x = varray[new A, 'func2$memoize_impl']; $x($v);
    $x = varray[new A, 'func3']; $x($v);
    $x = varray[new A, 'func3$memoize_impl']; $x($v);


    $x = 'A'; $x::func3($v);



    $x = 'func3'; A::$x($v);
    $x = 'func3$memoize_impl'; A::$x($v);

    $obj = new A; $x = 'func2'; $obj->$x($v);
    $obj = new A; $x = 'func2$memoize_impl'; $obj->$x($v);


  }
}

class B {
  <<__Memoize, __DynamicallyCallable>> public function func5($x) {}
  <<__Memoize, __DynamicallyCallable>> public static function func6($x) {}

  public static function positive_tests() {
    echo "====================== positive tests (B) ==================\n";
    $v = 123;

    $x = 'func4$memoize_impl'; $x($v);
    $x = 'B::func6$memoize_impl'; $x($v);

    //$x = varray['B', 'func5$memoize_impl']; $x($v); // fatal
    $x = varray['B', 'func6$memoize_impl']; $x($v);

    $x = varray[new B, 'func5$memoize_impl']; $x($v);
    $x = varray[new B, 'func6$memoize_impl']; $x($v);


    $x = 'func6$memoize_impl'; B::$x($v);

    $obj = new B; $x = 'func5$memoize_impl'; $obj->$x($v);

  }

  public static function negative_tests() {
    echo "====================== negative tests ======================\n";
    $v = 123;

    $x = 'func4'; $x($v);
    $x = 'B::func6'; $x($v);

    $x = varray['B', 'func6']; $x($v);

    $x = varray[new B, 'func5']; $x($v);
    $x = varray[new B, 'func6']; $x($v);

    $x = 'B'; $x::func6($v);

    $x = 'func6'; B::$x($v);

    $obj = new B; $x = 'func5'; $obj->$x($v);

  }
}
<<__EntryPoint>> function main(): void {
A::positive_tests();
B::positive_tests();
B::negative_tests();
}
