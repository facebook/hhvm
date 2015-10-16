<?hh

function string_key(): string {
  //UNSAFE
}

function test() {
  $a = array();
  $a[string_key()] = 4; // $a is Tarraykind AKempty, which we consider
  // partially checked
  $a; // $a is a <string, int> array - checked

  $b = array();
  $b[] = 4; // partially checked
  $b; // array<int> - checked

  $c = array();
  $c['a'] = 4; // partially checked (AKempty)
  $c['b'] = no_type(); // checked
  $c; // partially checked
}
