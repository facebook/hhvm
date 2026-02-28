<?hh
function main() :mixed{
  $containers = Vector {
    vec[11, 22, 33],
    Vector {11, 22, 33},
    Map {'a' => 11, 'b' => 22, 'c' => 33},
    Set {11, 22, 33},
    ImmVector {11, 22, 33},
    ImmMap {'a' => 11, 'b' => 22, 'c' => 33},
    ImmSet {11, 22, 33},
    Pair {11, 22},
    vec[],
    Vector {},
    Map {},
    Set {},
    ImmVector {},
    ImmMap {},
    ImmSet {},
  };
  foreach ($containers as $x) {
    var_dump(array_pop(inout $x));
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_array_pop() :mixed{
main();
}
