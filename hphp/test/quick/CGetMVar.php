<?hh

class V { const X = 10; }
function values() :mixed{
  $VALUES = dict[
    'X'                        => V::X,
  ];
  return $VALUES;
}
function foo() :mixed{ var_dump(values()['X']); }
<<__EntryPoint>> function main(): void {
foo();
foo();
}
