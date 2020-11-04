<?hh

class C {}

enum E : string {
  X = 'x';
  Y = 'y';
}

enum F : string {
  A = 'a';
  B = 'b';
}

function test($c) {
  return darray[
    E::X => darray[
      F::A => $c,
      F::B => 17,
    ],
    E::Y => 34,
  ];
}

<<__EntryPoint>>
function main() {
  $x = test(new C());
  print(HH\get_provenance($x)."\n");
  print(HH\get_provenance($x['x'])."\n");
}
