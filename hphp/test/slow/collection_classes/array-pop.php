<?hh
function main() {
  $containers = Vector {
    array(11, 22, 33),
    Vector {11, 22, 33},
    Map {'a' => 11, 'b' => 22, 'c' => 33},
    Set {11, 22, 33},
    ImmVector {11, 22, 33},
    ImmMap {'a' => 11, 'b' => 22, 'c' => 33},
    ImmSet {11, 22, 33},
    Pair {11, 22},
    array(),
    Vector {},
    Map {},
    Set {},
    ImmVector {},
    ImmMap {},
    ImmSet {},
  };
  foreach ($containers as $x) {
    var_dump(array_pop($x));
    var_dump($x);
  }
}
main();
