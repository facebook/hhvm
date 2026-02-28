<?hh

class A {
  public static $goo = 'foo';
}

function p() :mixed{
  print A::$goo . "\n";
}

function a() :mixed{
  $a = A::$goo;
  A::$goo = 'aoo';
  print $a . "\n";
}

function b() :mixed{
  $zoo = 'A';
  $b = $zoo::$goo;
  $zoo::$goo = 'boo';
  print $b . "\n";
}

<<__EntryPoint>>
function test() :mixed{
  if (__hhvm_intrinsics\launder_value(1)) {
    include 'sp.inc';
  }

  $a = B::$goo;
  print $a . "\n";
  B::$goo = 11;
  $a = B::$goo;
  print $a . "\n";
  p();
  a();
  p();
  b();
  p();
  print "round 2\n";
  a();
  p();
  A::$goo = 'foo';
  b();
  p();
  A::$goo = 'foo';
  p();
  A::$goo = 'foo';
  p();
}
