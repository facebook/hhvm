<?hh

function test($input) {
  $_ = $input->toArray();
  $_ = (array)$input;
}

<<__EntryPoint>>
function main() {
  $collections = vec[
    Vector {1, 2, 3},
    Set {'foo', 'bar'},
    Map {'foo' => 0, 'bar' => 0},
    Pair {'a', 'b'},
  ];
  foreach ($collections as $col) {
    test($col);
  }
}
