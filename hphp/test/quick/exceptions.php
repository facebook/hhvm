<?hh

class Ex1 extends Exception {
}

class Ex2 extends Exception {
}

class Ex3 extends Exception {
}

function bar($a) :mixed{
  print "bar $a\n";
  $b = vec[1, 2];
  foreach($b as $c) {
    if ($a == 0) {
      throw new Ex1();
    } else if ($a == 2) {
      throw new Ex2();
    } else if ($a == 3) {
      throw new Ex3();
    }
  }
}

function foo2($a) :mixed{
  try {
    call_user_func(bar<>, $a);
  } catch (Ex1 $e) {
    print "caught 1\n";
  }
}

function foo1($a) :mixed{
  foo2($a);
}

function foo($a) :mixed{
  foo1($a);
}

function main1() :mixed{
  $a = vec[0, 1, 2];
  $b = vec[0];

  foreach ($b as $c) {
    try {
      array_map(foo<>, $a);
    } catch (Ex2 $e) {
      print "caught 2\n";
    }
  }

  try {
    foreach (vec[1,2,3] as $_) {
      echo "before\n";
      throw new Exception();
      echo "after\n";
    }
  } catch (Exception $e) {
    echo "caught\n";
  }
}

class A {
  function __construct() {
    throw new Exception();
  }
}

function main2() :mixed{
  try {
    call_user_func(hphp_create_object<>, "A", NULL);
  } catch (Exception $e) {
    print "caught exception\n";
  }
}

class Ex4 extends Ex3 {
  function __construct($s) {
    var_dump($s);
    var_dump($this->getTraceAsString());
  }
}


function a() :mixed{
  return b();
}
function b() :mixed{
  return c();
}
function c() :mixed{
  $e = new Exception();
  var_dump($e->getTraceAsString());
  return new Ex4('hello, exception');
}
function main3() :mixed{
  $e = a();
  printf("Exception from %s:%d\n", $e->getFile(), $e->getLine());
  var_dump($e->getTraceAsString());

  $b = vec[3];
  try {
    array_map(foo<>, $b);
  } catch (Ex3 $e) {
    print "caught 3\n";
    throw $e;
  }
}

<<__EntryPoint>> function main(): void {
  main1();
  main2();
  main3();
}
