<?hh

function getData() {
  return Map {
    'a' => Pair{0.1, 2.3},
    'b' => Pair{4.5, 6.7},
    'c' => Pair{8.9, 2.4},
  };
}

function doMath() {
  $arr = getData();
  $sum = 0.0;
  foreach ($arr as $sample) {
    $sum += $sample[0] * $sample[1];
  }
  return $sum;
}

$total = 0.0;
for ($i = 0; $i < 200; ++$i) {
  $total += doMath();
}
var_dump($total);

function int_first() {
  $sum = 0;
  for ($i = 0; $i < 10; ++$i) {
    if (($i % 2) == 0) {
      $val = 2;
    } else {
      $val = 2.0;
    }
    $sum += $val;
  }
  return $sum;
}

function double_first() {
  $sum = 0.0;
  for ($i = 0; $i < 10; ++$i) {
    if (($i % 2) == 0) {
      $val = 2;
      $sum = 0;
    } else {
      $val = 2.0;
    }
    $sum += $val;
  }
  return $sum;
}

for ($i = 0; $i < 200; ++$i) {
  $total += int_first();
  $total += double_first();
}
var_dump($total);
