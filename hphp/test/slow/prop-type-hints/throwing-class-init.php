<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


function handler() :mixed{ throw new Exception('Error'); }

class A {
  public static int $x = 'abc';

  public static function get() :mixed{
    return A::$x;
  }
}

function test() :mixed{
  try {
    var_dump(A::get());
  } catch (Exception $e) {
    echo "Exception\n";
  }
}

<<__EntryPoint>> function main(): void {
set_error_handler(handler<>);
test();
}
