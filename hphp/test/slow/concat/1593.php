<?hh

class C {
  public function __toString() {
    return 'bar';
  }
}
function f($x) {
  var_dump($x . '');
}

<<__EntryPoint>>
function main_1593() {
f(123);
f(new C);
}
