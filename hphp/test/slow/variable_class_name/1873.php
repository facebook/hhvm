<?hh

function func() :mixed{
 return 'B';
}
class B {
  function foo() :mixed{
 var_dump(__CLASS__);
}
  function f4missing() :mixed{
 $this->foo();
}
}
class G extends B {
  function foo() :mixed{
 var_dump(__CLASS__);
}
  function f4missing() :mixed{
 $b = func();
 $b::f4missing();
}
}

<<__EntryPoint>>
function main_1873() :mixed{
$g = new G;
 $g->f4missing();
}
