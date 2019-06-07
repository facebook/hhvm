<?hh /* destructor */
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls {
  function func() { echo "func called\n"; }
  static function intercept1() { return false; }
  static function intercept2($a, $b, $c, $d, $e) {
    var_dump(debug_backtrace());
    return false;
  }
  static function intercept3() { throw new Exception("intercept3"); }
}

function getCls() { return new Cls; }

function test($s) {
  fb_intercept('Cls::func', "Cls::$s");
  try {
    getCls()->func();
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
  echo "$s DONE\n";
}

test('intercept1');
test('intercept2');
test('intercept3');
