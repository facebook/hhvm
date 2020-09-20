<?hh

class C {}

function bad_math(dynamic $dyn, C $c): void {
  if($dyn) {
    $c = $dyn;
  }
  $c + 5; // error
  $c - 5; // error
  $c * 5; // error
  $c / 5; // error
  $c ** 5; // error
  ~$c; // error
  $c % 0; // error
  $c << 0; // error
  $c >> 0; // error
  $c ^ 0; // error
  $c & 0; // error
  $c | 0; // error
  +$c; // error
  -$c; // error
}
