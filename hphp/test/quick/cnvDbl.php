<?hh

function foo($v) :mixed{
  return (float)$v;
}
function foo2(): void {
  var_dump(foo(null));
  var_dump(foo(true));
  var_dump(foo(1));
  var_dump(foo(9007199254740992));
  var_dump(foo(1.1));
  var_dump(foo("abc"));
  var_dump(foo(vec[123]));
  try {
    var_dump(foo(new C));
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
}
class C { public $foo = "bar"; }
function real($i) :mixed{
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
    $v10 = vec[];
    $v11 = vec[123];
  }
  var_dump((float)$v1);
  var_dump((float)$v2);
  var_dump((float)$v3);
  var_dump((float)$v4);
  var_dump((float)$v5);
  var_dump((float)$v6);
  var_dump((float)$v7);
  var_dump((float)$v8);
  try {
    var_dump((float)$v9);
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  var_dump((float)$v10);
  var_dump((float)$v11);
}

function baz($i) :mixed{
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
