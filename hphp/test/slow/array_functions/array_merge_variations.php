<?hh

class C {
  public static function f(): void { }
}

<<__EntryPoint>> function main(): void {
  // we are testing edge cases around input types and key types in the input
  // so the values in these just need to be distinct
  $values = dict[
    'vec' => vec[1],
    'dict' => dict[2 => 2, '2' => '2', 3 => 3],
    'keyset' => keyset[4, '4', 5],
    'Vector' => Vector{6},
    'Map' => Map{7 => 7, '7' => '7', 8 => 8},
    'Set' => Set{9, '9', 10},
    'ImmVector' => ImmVector{11},
    'ImmMap' => ImmMap{12 => 12, '12' => '12', 13 => 13},
    'ImmSet' => ImmSet{14, '14', 15},
    'Pair' => Pair{16, 17},
    'class_meth' => C::f<>,
    'false' => false, // any value not resembling a Container
  ];

  foreach ($values as $n1 => $v1) {
    foreach ($values as $n2 => $v2) {
      echo "==== array_merge($n1, $n2) ====\n";
      try {
        var_dump(array_merge($v1, $v2));
      } catch (Exception $e) {
        echo var_dump($e->getMessage());
      }
    }
  }
}
