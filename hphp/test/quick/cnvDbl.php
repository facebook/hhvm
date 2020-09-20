<?hh

function foo($v) {
  return (float)$v;
}
function foo2(): void {
  var_dump(foo(null));
  var_dump(foo(true));
  var_dump(foo(1));
  var_dump(foo(9007199254740992));
  var_dump(foo(1.1));
  var_dump(foo("abc"));
  var_dump(foo(varray[123]));
  var_dump(foo(new C));
}
class C { public $foo = "bar"; }
function real($i) {
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
    $v10 = varray[];
    $v11 = varray[123];
  }
  var_dump((float)$v1);
  var_dump((float)$v2);
  var_dump((float)$v3);
  var_dump((float)$v4);
  var_dump((float)$v5);
  var_dump((float)$v6);
  var_dump((float)$v7);
  var_dump((float)$v8);
  var_dump((float)$v9);
  var_dump((float)$v10);
  var_dump((float)$v11);
}

function baz($i) {
  if ($i >= 1) {
    $s1 = '5.3xxx';
    $s2 = '7yyy';
  }
  var_dump((float)$s1);
  var_dump((float)$s2);
}

<<__EntryPoint>> function main(): void {
foo2();
real(1);
baz(1);
}
