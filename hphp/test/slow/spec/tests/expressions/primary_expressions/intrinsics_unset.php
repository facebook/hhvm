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
    unset($p);
    var_dump(isset($p));
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

function f($p) :mixed{
  var_dump($p);
  var_dump(isset($p));
  unset($p);
  var_dump(isset($p));
}

class X1 {
}

function g2() :mixed{
  var_dump(\HH\global_isset('gl'));
  \HH\global_unset('gl'); // unsets global "version"
  var_dump(\HH\global_isset('gl'));
}

function g3($p1, inout $p2) :mixed{
  try {
    var_dump(isset($p1, $p2));
    unset($p1, $p2); // unsets local "version" in current scope
    var_dump(isset($p1, $p2));
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

class State {
  public static $count = 0;
}
function g4() :mixed{
  ++State::$count;
  echo "count = ".State::$count."\n";

  var_dump(isset(State::$count));
  var_dump(false);
}

class C {
  const CON1 = 123;
  public $prop = 10;
  public static $sprop = -5;
}

<<__EntryPoint>>
function entrypoint_intrinsics_unset(): void {
  error_reporting(-1);

  echo "--------- TRUE -------------\n";

  $v = TRUE;
  var_dump(isset($v));
  unset($v);
  var_dump(isset($v));

  echo "--------- NULL -------------\n";

  $v = NULL;
  var_dump(isset($v));
  unset($v);
  var_dump(isset($v));

  echo "--------- TRUE, 12.3, NULL -------------\n";

  $v1 = TRUE;
  $v2 = 12.3;
  $v3 = "abc";
  var_dump(isset($v1, $v2, $v3));
  unset($v1, $v2, $v3);
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
  unset($x1->m);
  var_dump(isset($x1->m));

  echo "---------- unsetting inside a function (\$GLOBALS) ------------\n";

  \HH\global_set('gl', 100);
  var_dump(\HH\global_isset('gl')); // still set

  g2();
  var_dump(\HH\global_isset('gl')); // no longer set

  echo "---------- unsetting inside a function (pass-by-inout) ------------\n";

  $v1 = 10;
  $v2 = 20;
  try {
    g3($v1, inout $v2);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  var_dump(isset($v1)); // still set
  var_dump($v1);
  var_dump(isset($v2)); // no longer set
  var_dump($v2);

  echo "---------- unsetting inside a function (static) ------------\n";

  g4();
  g4();

  echo "---------- unsetting a property ------------\n";

  $c1 = new C;
  var_dump($c1);
  var_dump(isset($c1->prop));
  unset($c1->prop); // remove it from this instance
  var_dump(isset($c1->prop));

  //unset(C::$sprop);         // Attempt to unset static property

  var_dump($c1);

  echo "-----------\n";

  $c2 = new C;
  var_dump($c2);
  var_dump(isset($c2->prop));

  echo "---------- unsetting array elements ------------\n";

  $a = dict[0 => 10, 1 => 20, "xx" => 30];
  print_r($a);
  unset($a[1]);
  print_r($a);

  unset($a[10]);
  print_r($a);

  unset($a["Xx"]);
  print_r($a);
}
