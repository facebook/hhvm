<?hh

function func() {
 return 'B';
}
class B {
  function foo() {
 var_dump(__CLASS__);
}
  function f4missing() {
 $this->foo();
}
}
class G extends B {
  function foo() {
 var_dump(__CLASS__);
}
  function f4missing() {
 $b = func();
 $b::f4missing();
}
}

<<__EntryPoint>>
function main_1873() {
$g = new G;
 $g->f4missing();
}
