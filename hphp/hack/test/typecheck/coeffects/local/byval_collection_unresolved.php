<?hh

function succeed<Tv as arraykey>(bool $c, Tv $v1, Tv $v2)[]: keyset<Tv> {
  $result = keyset[];
  if ($c) {
    $result[] = $v1;
  } else {
    $result[] = $v2;
  }
  $result[] = $v1; // OK
  return $result;
}

function fail<Tv as arraykey>(bool $c, Tv $v)[]: mixed {
  $result = vec[];
  if ($c) {
    $result[] = $v;
  } else {
    $result = Vector { $v };
  }
  $result[0] = $v; // ERROR
  return $result;
}

function succeed_when_WriteProperty<Tv as arraykey>(bool $c, Tv $v)[
  \HH\Capabilities\WriteProperty
]: mixed {
  $result = vec[];
  if ($c) {
    $result[] = $v;
  } else {
    $result = Vector { $v };
  }
  $result[0] = $v; // OK (has WriteProperty)
  return $result;
}
