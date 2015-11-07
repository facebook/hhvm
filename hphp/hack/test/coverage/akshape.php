<?hh

function string_key(): string {
  //UNSAFE
}

function test() {
  $a = array();
  $a[string_key()] = 4; // $a is updated to AKmap, checked
  $a; // $a is a <string, int> array - checked

  $b = array();
  $b[] = 4; // still AKempty, checked
  $b; // array<int> - checked

  $c = array();
  $c['a'] = 4; // checked (AKmap)
  $c['b'] = no_type(); // checked
  $c; // partially checked
}
