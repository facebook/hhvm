<?hh

class main {}

function foo(Stringish $s) {
  var_dump($s);
}

<<__EntryPoint>>
function main() {
  foo('hello');
  foo(__hhvm_intrinsics\create_class_pointer('main'));
}
