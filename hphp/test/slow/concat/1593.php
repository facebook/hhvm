<?hh

class C {
  public function __toString()[] :mixed{
    return 'bar';
  }
}
function f($x) :mixed{
  var_dump($x . '');
}

<<__EntryPoint>>
function main_1593() :mixed{
f(123);
f(new C);
}
