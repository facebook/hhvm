<?hh

class Bar {}

function bar() :mixed{
  // Tests HHBBC type system
  var_dump(is_string(Bar::class));
  var_dump(Bar::class is string);
  var_dump(Bar::class is arraykey);
  var_dump(HH\is_class(Bar::class));
}

function foo($c) :mixed{
  var_dump(is_string($c));
  $a = $c is string;
  var_dump($a);
  $a = $c is arraykey;
  var_dump($a);
  var_dump(HH\is_class($c));
}

<<__EntryPoint>>
function main() :mixed{
  foo(Bar::class);
  foo(__hhvm_intrinsics\launder_value(Bar::class));
  bar();
}
