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
}
