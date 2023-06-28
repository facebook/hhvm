<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function e() :mixed{
  try {
    var_dump($p);
    var_dump(isset($p));
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

function f($p) :mixed{
  var_dump($p);
  var_dump(isset($p));
}

class X1 {
}
<<__EntryPoint>>
function entrypoint_intrinsics_isset(): void {
  error_reporting(-1);

  echo "--------- TRUE -------------\n";

  $v = TRUE;
  var_dump(isset($v));

  echo "--------- NULL -------------\n";

  $v = NULL;
  var_dump(isset($v));

  echo "--------- TRUE, 12.3, NULL -------------\n";

  $v1 = TRUE;
  $v2 = 12.3;
  $v3 = NULL;
  var_dump(isset($v1, $v2, $v3));

  echo "--------- undefined parameter -------------\n";

  e();
  f(NULL);
  f(10);

  echo "---------- dynamic property ------------\n";

  $x1 = new X1;
  var_dump(isset($x1->m));
  $x1->m = 123;
  var_dump(isset($x1->m));
}
