<?hh
// Copyright 2004-present Facebook. All Rights Reserved.



<<__Memoize>> function func1($x) :mixed{}
<<__Memoize, __DynamicallyCallable>> function func4($x) :mixed{}

class A {
  <<__Memoize>> public function func2($x) :mixed{}
  <<__Memoize>> public static function func3($x) :mixed{}

  public static function positive_tests() :mixed{
    echo "====================== positive tests (A) ==================\n";
    $v = 123;

    $x = 'func1'; $x($v);
    $x = 'func1$memoize_impl'; $x($v);
    $x = 'func4$memoize_impl'; $x($v);
    //$x = 'A::func2'; $x($v); // fatal
    //$x = 'A::func2$memoize_impl'; $x($v); // fatal
    $x = 'A::func3'; $x($v);
    $x = 'A::func3$memoize_impl'; $x($v);

    //$x = vec['A', 'func2']; $x($v); // fatal
    //$x = vec['A', 'func2$memoize_impl']; $x($v); // fatal
    $x = vec['A', 'func3']; $x($v);
    $x = vec['A', 'func3$memoize_impl']; $x($v);

    $x = vec[new A, 'func2']; $x($v);
    $x = vec[new A, 'func2$memoize_impl']; $x($v);
    $x = vec[new A, 'func3']; $x($v);
    $x = vec[new A, 'func3$memoize_impl']; $x($v);


    $x = 'A'; $x::func3($v);



    $x = 'func3'; A::$x($v);
    $x = 'func3$memoize_impl'; A::$x($v);

    $obj = new A; $x = 'func2'; $obj->$x($v);
    $obj = new A; $x = 'func2$memoize_impl'; $obj->$x($v);


  }
}

class B {
  <<__Memoize, __DynamicallyCallable>> public function func5($x) :mixed{}
  <<__Memoize, __DynamicallyCallable>> public static function func6($x) :mixed{}

  public static function positive_tests() :mixed{
    echo "====================== positive tests (B) ==================\n";
    $v = 123;

    $x = 'func4$memoize_impl'; $x($v);
    $x = 'B::func6$memoize_impl'; $x($v);

    //$x = vec['B', 'func5$memoize_impl']; $x($v); // fatal
    $x = vec['B', 'func6$memoize_impl']; $x($v);

    $x = vec[new B, 'func5$memoize_impl']; $x($v);
    $x = vec[new B, 'func6$memoize_impl']; $x($v);


    $x = 'func6$memoize_impl'; B::$x($v);

    $obj = new B; $x = 'func5$memoize_impl'; $obj->$x($v);

  }

  public static function negative_tests() :mixed{
    echo "====================== negative tests ======================\n";
    $v = 123;

    $x = 'func4'; $x($v);
    $x = 'B::func6'; $x($v);

    $x = vec['B', 'func6']; $x($v);

    $x = vec[new B, 'func5']; $x($v);
    $x = vec[new B, 'func6']; $x($v);

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
