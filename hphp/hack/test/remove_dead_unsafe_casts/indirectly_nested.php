<?hh

function foo(arraykey $x): int {
  return 0;
}

function bar(string $x): num {
  return HH\FIXME\UNSAFE_CAST<mixed,num>(foo(HH\FIXME\UNSAFE_CAST<mixed,arraykey>($x)));
}
