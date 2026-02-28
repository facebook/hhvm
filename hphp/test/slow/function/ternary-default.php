<?hh

function f($val = (MY_CONST === 123) ? "Foo" : "Bar") :mixed{
  echo "val = ";
  var_dump($val);
}

const MY_CONST = 123;

<<__EntryPoint>>
function main_ternary_default() :mixed{

f();
}
