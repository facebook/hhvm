<?hh

class X {
}
class Y extends X {
 public $foo;
 }
function foo() :mixed{
  $x = new Y;
  $x && var_dump($x->foo);
  $x = new X;
  var_dump($x);
}

<<__EntryPoint>>
function main_1305() :mixed{
foo();
}
