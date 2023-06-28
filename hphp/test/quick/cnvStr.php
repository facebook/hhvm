<?hh

function foo($v) :mixed{
  return (string)$v;
}
class C { public function __toString() :mixed{ return "a C"; } }

function bar($i) :mixed{
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
    $v5 = 1;
    $v6 = 0.0;
    $v7 = 1.1;
    $v8 = "abc";
    $v9 = new C();
  }
  var_dump((string)$v1);
  var_dump((string)$v2);
  var_dump((string)$v3);
  var_dump((string)$v4);
  var_dump((string)$v5);
  var_dump((string)$v6);
  var_dump((string)$v7);
  var_dump((string)$v8);
  var_dump((string)$v9);
}

class D { }
<<__EntryPoint>>
function entrypoint_cnvStr(): void {
  var_dump(foo(null));
  var_dump(foo(false));
  var_dump(foo(true));
  var_dump(foo(1));
  var_dump(foo(1.1));
  var_dump(foo("abc"));
  var_dump(foo(new C));

  bar(1);
  var_dump(foo(new D));
}
