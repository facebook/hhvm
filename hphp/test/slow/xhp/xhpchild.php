<?php

class :div implements XHPChild {}

function f1(?XHPChild $x): void {
  echo (int)($x instanceof XHPChild), ", ";
  var_dump($x);
  echo "\n";
}

function f2(XHPChild $x): void {
  echo (int)($x instanceof XHPChild), ", ";
  var_dump($x);
  echo "\n";
}

function main() {
  // check array
  f1(array(1,2,3,4));
  f2(array(1,2,3,4));

  // check a static string
  f1("a boring string");
  f2("a boring string");

  // check a dynamic string
  $x = "hello ";
  if (time() > 0) {
    $x .= "world";
  }
  f1($x);
  f2($x);

  // check numerics
  f1(10);
  f2(10);
  f1(-4.2);
  f2(-4.2);

  // check an xhp element
  f1(<div>hello world</div>);
  f2(<div>hello world</div>);

  // null
  f1(null);
  f2(null);
}
main();
