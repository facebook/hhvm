<?hh

<<__DynamicallyReferenced>> class main {}

function foo(Stringish $s) :mixed{
  var_dump($s);
}

function fooObj(StringishObject $s) :mixed{
  var_dump($s);
}

<<__EntryPoint>>
function main() :mixed{
  foo('hello');
  foo(HH\classname_to_class('main'));
  fooObj(HH\classname_to_class('main'));
}
