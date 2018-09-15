<?hh

function foo($v) {
  return (double)$v;
}

var_dump(foo(null));
var_dump(foo(true));
var_dump(foo(1));
var_dump(foo(9007199254740992));
var_dump(foo(1.1));
var_dump(foo("abc"));
var_dump(foo(array(123)));
class C { public $foo = "bar"; }
var_dump(foo(new C));

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
  if ($i >= 1) {
    $v1 = null;
    $v2 = false;
    $v3 = true;
    $v4 = 0;
    $v5 = "1";
    $v6 = 0.0;
    $v7 = "1.1";
    $v8 = "abc";
    $v9 = new C;
    $v10 = array();
    $v11 = array(123);
  }
  var_dump((double)$v1);
  var_dump((double)$v2);
  var_dump((double)$v3);
  var_dump((double)$v4);
  var_dump((double)$v5);
  var_dump((double)$v6);
  var_dump((double)$v7);
  var_dump((double)$v8);
  var_dump((double)$v9);
  var_dump((double)$v10);
  var_dump((double)$v11);
}

bar(1);

function baz($i) {
  if ($i >= 1) {
    $s1 = '5.3xxx';
    $s2 = '7yyy';
  }
  var_dump((double)$s1);
  var_dump((double)$s2);
}

baz(1);
