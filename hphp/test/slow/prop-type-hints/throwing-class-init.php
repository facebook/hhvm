<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


function handler() { throw new Exception('Error'); }
set_error_handler('handler');

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

test();
