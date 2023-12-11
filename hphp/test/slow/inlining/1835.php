<?hh

function id($x) :mixed{
 return $x;
 }
class B {
 function __construct($x) {
 $this->x = $x;
 }
 }
class X extends B {
  function __construct() {
 parent::__construct(vec[]);
 }
  function foo() :mixed{
 echo "foo
";
 }
}
function bar($x=0) :mixed{
 if ($x) return 1;
 return '';
 }
function test($foo) :mixed{
  id(new X(bar()))->foo();
  id(new $foo(bar()))->foo();
}

<<__EntryPoint>>
function main_1835() :mixed{
test('X');
}
