<?hh

class C {
  const using = 'in C';
}



<<__EntryPoint>>
function main_names() :mixed{
var_dump(C::using);

$using = 'local variable';
var_dump($using);
}
