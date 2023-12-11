<?hh

class Foo {
  public $a;
  static public $b;
  static public $c;
}

<<__EntryPoint>>
function main_1851() :mixed{
$foo = new Foo;
$foo->a = function ($x) {
 echo '!' . $x;
 }
;
($foo->a)("foo\n");
Foo::$b = function ($x) {
 echo '?' . $x;
 }
;
(Foo::$b)("bar\n");
Foo::$c = vec[function ($x) {
 echo '.' . $x;
}]
;
(Foo::$c[0])("baz\n");
}
