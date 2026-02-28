<?hh
<<__DynamicallyConstructible>>
class B1 {
}
<<__DynamicallyConstructible>>
class C1 {
 function __construct() {
}
 }
<<__DynamicallyConstructible>>
class D1 {
 }
<<__DynamicallyConstructible>>
class D2 extends C1 {
 }
<<__DynamicallyConstructible>>
class D3 extends D2 {
}
<<__DynamicallyConstructible>>
class D4 extends B1 {
 }
<<__DynamicallyConstructible>>
class D5 extends D4 {
}
<<__DynamicallyConstructible>>
class D6 extends D1 {
 function __construct($a) {
 if ($a) f();
 }
 }
function f() :mixed{
 throw new Exception('throw');
 }
function foo($a,$b) :mixed{
  try {
    $x = new D6($b?f():$a);
  }
 catch (Exception $e) {
    var_dump('caught');
  }
}
function bar($x, $a, $b) :mixed{
  try {
    $x = new $x($b?f():$a);
  }
 catch (Exception $e) {
    var_dump('caught');
  }
}
function n($x, ...$args) :mixed{
 return new $x(...$args);
 }
function baz($d) :mixed{
  $x = new D1;
  $x = new D2;
  $x = new D3;
  $x = new D4;
  $x = new D5;
  $x = new D6(false);
  $x = n($d.'1');
  $x = n($d.'2');
  $x = n($d.'3');
  $x = n($d.'4');
  $x = n($d.'5');
  $x = n($d.'6', false);
  $x = n('B1');
}


<<__EntryPoint>>
function main_1570() :mixed{
;
foo(false,false);
foo(false,true);
foo(true,true);
foo(true,false);
bar('D6',false,false);
bar('D6',false,true);
bar('D6',true,false);
bar('D6',true,true);
baz('D');
}
