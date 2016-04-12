<?hh

function no_type() {}

function test(): void {
  $s = shape();
  $s['a'] = 4; // $s in this expression is a shape with no fields - checked
  $s['b'] = no_type(); // $s is a shape with one, known, field - checked
  $s; // $shape with a field of unknown type, partially checked
}
