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

    $empty_col_1 = Set {};
    $empty_col_2 = StableMap {};
    $empty_col_3 = Vector {};
    $empty_col_4 = Map {};
    $empty_col_5 = ImmVector {};
    $empty_col_6 = ImmSet {};
    $empty_col_7 = ImmMap {};

    $filled_col_1 = Set {1};
    $filled_col_2 = StableMap {'k' => 'v'};
    $filled_col_3 = Vector {1};
    $filled_col_4 = Map {'k' => 'v'};
    $filled_col_5 = ImmVector {1};
    $filled_col_6 = Set {1};
    $filled_col_7 = Map {'k' => 'v'};
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

  echo "= collections =\n";
  var_dump((bool) $empty_col_1);
  var_dump((bool) $empty_col_2);
  var_dump((bool) $empty_col_3);
  var_dump((bool) $empty_col_4);
  var_dump((bool) $empty_col_5);
  var_dump((bool) $empty_col_6);
  var_dump((bool) $empty_col_7);
  var_dump((bool) $filled_col_1);
  var_dump((bool) $filled_col_2);
  var_dump((bool) $filled_col_3);
  var_dump((bool) $filled_col_4);
  var_dump((bool) $filled_col_5);
  var_dump((bool) $filled_col_6);
  var_dump((bool) $filled_col_7);
}

bar(1);
?>
