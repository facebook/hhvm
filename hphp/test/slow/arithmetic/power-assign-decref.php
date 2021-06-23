<?hh

class D {
}

function main() {
  $i1 = new D;
  $i2 = new D;
  $i3 = new D;
  $i4 = new D;
  $i5 = new D;
  $i6 = new D;
  $i7 = new D;
  $i8 = new D;
  $i9 = new D;

  try {
    $i1 += 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i2 -= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i3 *= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i4 /= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i5 %= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i6 **= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i7 &= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i8 |= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  try {
    $i9 ^= 1;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }

  var_dump($i1);
  var_dump($i2);
  var_dump($i3);
  var_dump($i4);
  var_dump($i5);
  var_dump($i6);
  var_dump($i7);
  var_dump($i8);
  var_dump($i9);
}


<<__EntryPoint>>
function main_power_assign_decref() {
error_reporting(E_NOTICE);

main();
}
