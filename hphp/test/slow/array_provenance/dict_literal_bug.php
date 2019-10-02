<?hh

class Foo {
  const GARBAGE = 100;
}

<<__EntryPoint>>
function main() {
  $x = dict[
    42 => "garbage",
    Foo::GARBAGE => "more garbage"
  ];

  var_dump(HH\get_provenance($x));
}
