<?php

function foo($v) {
  return (bool)$v;
}

var_dump(foo(null));
var_dump(foo(true));
var_dump(foo(1));
var_dump(foo(1.1));
var_dump(foo("true"));
var_dump(foo(array(123)));

class C{}

function bar($i) {
  $v1 = "undefined";
  $v2 = "undefined";
  $v3 = "undefined";
  $v4 = "undefined";
  $v5 = "undefined";
  $v6 = "undefined";
  $v7 = "undefined";
  $v8 = "undefined";
  $v9 = "undefined";
  $v10 = "undefined";
  $v11 = "undefined";
  $v12 = "undefined";
  $v13 = "undefined";
  $v14 = "undefined";
  $v15 = "undefined";
  if ($i >= 1) {
    $v1 = null;
    $v2 = false;
    $v3 = true;
    $v4 = 0;
    $v5 = 1;
    $v6 = 0.0;
    $v7 = 1.1;
    $v8 = "false";
    $v9 = new C();
    $v10 = array();
    $v11 = array(1);

    $vec_1 = Set {};
    $vec_2 = StableMap {};
    $vec_3 = Vector {};
    $vec_4 = Map {};
    $vec_5 = FrozenVector {};

    $vfc_1 = Set {1};
    $vfc_2 = StableMap {'k' => 'v'};
    $vfc_3 = Vector {1};
    $vfc_4 = Map {'k' => 'v'};
    $vfc_5 = FrozenVector {1};
  }
  var_dump((bool) $v1);
  var_dump((bool) $v2);
  var_dump((bool) $v3);
  var_dump((bool) $v4);
  var_dump((bool) $v5);
  var_dump((bool) $v6);
  var_dump((bool) $v7);
  var_dump((bool) $v8);
  var_dump((bool) $v9);
  var_dump((bool) $v10);
  var_dump((bool) $v11);

  var_dump((bool) $vec_1);
  var_dump((bool) $vec_2);
  var_dump((bool) $vec_3);
  var_dump((bool) $vec_4);
  var_dump((bool) $vec_5);
  var_dump((bool) $vfc_1);
  var_dump((bool) $vfc_2);
  var_dump((bool) $vfc_3);
  var_dump((bool) $vfc_4);
  var_dump((bool) $vfc_5);
}

bar(1);
?>
