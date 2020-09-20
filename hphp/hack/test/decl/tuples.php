<?hh

type Coordinate = (float, float);

function takes_tuple((float, float) $arg1): Coordinate {
  return $arg1;
}

function returns_tuple(Coordinate $arg1): (float, float) {
  return $arg1;
}

function generic_tuple<T>((T, T) $arg1): (T, T) {
  return $arg1;
}
