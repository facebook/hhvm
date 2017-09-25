<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls {
  function func() { echo "func called\n"; }
  function intercept1() { return false; }
  function intercept2() {
    var_dump(debug_backtrace());
    return false;
  }
  function intercept3() { throw new Exception("intercept3"); }
  function __destruct() {}
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
