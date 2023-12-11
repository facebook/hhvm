<?hh

function run($fn) :mixed{
  try {
    print(json_encode($fn())."\n");
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

function set0($x): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[0] = 17;
  return $result;
}

function set1($x): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[1] = 17;
  return $result;
}

function setNew($x): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[] = 17;
  return $result;
}

function setInt($x, int $y): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[$y] = 17;
  return $result;
}

function setStr($x, string $y): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[$y] = 17;
  return $result;
}

function setCell($x, $y): varray<int> {
  $result = vec[];
  for ($i = 0; $i < $x; $i++) {
    $result[] = $i * $i;
  }
  $result[$y] = 17;
  return $result;
}

function set0Tuple($x): varray<int> {
  $result = vec[0, 1, $x];
  $result[0] = 17;
  return $result;
}

function set3Tuple($x): varray<int> {
  $result = vec[0, 1, $x];
  $result[3] = 17;
  return $result;
}

<<__EntryPoint>>
function main() :mixed{
  run(() ==> set0(3));
  run(() ==> set1(3));
  run(() ==> setNew(3));
  run(() ==> setInt(3, 2));
  run(() ==> setInt(3, 3));
  run(() ==> setStr(3, '3'));
  run(() ==> setCell(3, 2));
  run(() ==> setCell(3, 3));
  run(() ==> set0Tuple(3));
  run(() ==> set3Tuple(3));
}
