<?php

error_reporting(-1);

$x = (int)apc_fetch('foo');
apc_store('foo', $x + 1);

class X {
  function bar() { var_dump(__METHOD__); }
}

function f() { var_dump(__METHOD__); }
ClosureStaticInvoke::$f = function ($x) { var_dump(__METHOD__); };

function foo($x) {

  if ($x == 0) {
    echo "statically known invoke\n";
    $g = function ($x) { var_dump(__METHOD__); };
    $g::__invoke(f());
  } else if ($x == 1) {
    echo "call_user_func\n";
    call_user_func(array(get_class(ClosureStaticInvoke::$f), "__invoke"), f());
  } else {
    echo "X::bar\n";
    X::bar();
    echo "cuf(X::bar)\n";
    call_user_func(array('X', 'bar'));
    echo "statically unknown invoke\n";
    (ClosureStaticInvoke::$f)::__invoke(f());
  }
}

foo($x);

abstract final class ClosureStaticInvoke {
  public static $f;
}
