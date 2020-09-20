<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function wrap($e) { echo "Exception: {$e->getMessage()}\n"; }

<<__Memoize>> function func1($x) {}
<<__Memoize, __DynamicallyCallable>> function func4($x) {}

class A {
  <<__Memoize>> public function func2($x) {}
  <<__Memoize>> public static function func3($x) {}

  public static function positive_tests() {
    echo "====================== positive tests (A) ==================\n";
    $v = 123;

    try { $x = 'func1'; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = 'func1$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = 'func4$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); }
    //try { $x = 'A::func2'; $x($v); } catch (Exception $e) { wrap($e); } // fatal
    //try { $x = 'A::func2$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); } // fatal
    try { $x = 'A::func3'; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = 'A::func3$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); }

    //try { $x = varray['A', 'func2']; $x($v); } catch (Exception $e) { wrap($e); } // fatal
    //try { $x = varray['A', 'func2$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); } // fatal
    try { $x = varray['A', 'func3']; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = varray['A', 'func3$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }

    try { $x = varray[new A, 'func2']; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = varray[new A, 'func2$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = varray[new A, 'func3']; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = varray[new A, 'func3$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }


    try { $x = 'A'; $x::func3($v); } catch (Exception $e) { wrap($e); }



    try { $x = 'func3'; A::$x($v); } catch (Exception $e) { wrap($e); }
    try { $x = 'func3$memoize_impl'; A::$x($v); } catch (Exception $e) { wrap($e); }

    try { $obj = new A; $x = 'func2'; $obj->$x($v); } catch (Exception $e) { wrap($e); }
    try { $obj = new A; $x = 'func2$memoize_impl'; $obj->$x($v); } catch (Exception $e) { wrap($e); }


  }
}

class B {
  <<__Memoize, __DynamicallyCallable>> public function func5($x) {}
  <<__Memoize, __DynamicallyCallable>> public static function func6($x) {}

  public static function positive_tests() {
    echo "====================== positive tests (B) ==================\n";
    $v = 123;

    try { $x = 'func4$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = 'B::func6$memoize_impl'; $x($v); } catch (Exception $e) { wrap($e); }

    //try { $x = varray['B', 'func5$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); } // fatal
    try { $x = varray['B', 'func6$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }

    try { $x = varray[new B, 'func5$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }
    try { $x = varray[new B, 'func6$memoize_impl']; $x($v); } catch (Exception $e) { wrap($e); }


    try { $x = 'func6$memoize_impl'; B::$x($v); } catch (Exception $e) { wrap($e); }

    try { $obj = new B; $x = 'func5$memoize_impl'; $obj->$x($v); } catch (Exception $e) { wrap($e); }

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
