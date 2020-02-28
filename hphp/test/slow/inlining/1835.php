<?hh

function id($x) {
 return $x;
 }
class B {
 function __construct($x) {
 $this->x = $x;
 }
 }
class X extends B {
  function __construct() {
 parent::__construct(varray[]);
 }
  function foo() {
 echo "foo
";
 }
}
function bar($x=0) {
 if ($x) return 1;
 return '';
 }
function test($foo) {
  id(new X(bar()))->foo();
  id(new $foo(bar()))->foo();
}

<<__EntryPoint>>
function main_1835() {
test('X');
}
