<?hh

class A {
  public static $goo = 'foo';
}

function p() {
  print A::$goo . "\n";
}

function a() {
  $a = A::$goo;
  A::$goo = 'aoo';
  print $a . "\n";
}

function b() {
  $zoo = 'A';
  $b = $zoo::$goo;
  $zoo::$goo = 'boo';
  print $b . "\n";
}

<<__EntryPoint>>
function test() {
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
