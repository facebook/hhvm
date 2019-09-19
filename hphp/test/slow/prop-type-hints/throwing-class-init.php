<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


function handler() { throw new Exception('Error'); }

class A {
  public static int $x = 'abc';

  public static function get() {
    return A::$x;
  }
}

function test() {
  try {
    var_dump(A::get());
  } catch (Exception $e) {
    echo "Exception\n";
  }
}

<<__EntryPoint>> function main(): void {
set_error_handler(fun('handler'));
test();
}
