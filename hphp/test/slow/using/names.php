<?hh

class C {
  const using = 'in C';
}

var_dump(C::using);

$using = 'local variable';
var_dump($using);

