<?php

class X {
  static function foo() {
    var_dump(get_called_class());
    return function() {
      var_dump(get_called_class());
    };
  }
  function bar() {
    var_dump(get_called_class());
    return static function() {
      var_dump(get_called_class());
    };
  }
  function bar_nonstatic() {
    var_dump(get_called_class());
    return function() {
      var_dump(get_called_class());
    };
  }
}
class Y extends X {}

function test() {
  $a = X::foo();
  $a();
  $a = Y::foo();
  $a();

  $a = X::bar();
  $a();
  $a = Y::bar();
  $a();

  $x = new X;
  $a = $x->bar();
  $a();
  $x = new Y;
  $a = $x->bar();
  $a();

  $a = X::bar_nonstatic();
  $a();
  $a = Y::bar_nonstatic();
  $a();

  $x = new X;
  $a = $x->bar_nonstatic();
  $a();
  $x = new Y;
  $a = $x->bar_nonstatic();
  $a();
}

test();
