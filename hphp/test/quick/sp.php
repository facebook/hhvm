<?
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

function c() {
  $yoo = 'goo';
  $c = A::$$yoo;
  A::$$yoo = 'coo';
  print $c . "\n";
}

function d() {
  $zoo = 'A';
  $yoo = 'goo';
  $d = $zoo::$$yoo;
  $zoo::$$yoo = 'doo';
  print $d . "\n";
}

if (1) {
  class B {
    public static $goo = 'bozo';
  }
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
c();
p();
d();
p();
print "round 2\n";
a();
p();
A::$goo = 'foo';
b();
p();
A::$goo = 'foo';
c();
p();
A::$goo = 'foo';
d();
p();
?>
