<?hh

type Coordinate = shape('x' => float, 'y' => float, ...);

function takes_shape(shape('x' => float, 'y' => float) $arg1): Coordinate {
  return $arg1;
}

function returns_shape(
  Coordinate $arg1,
): shape('x' => float, 'y' => float, ...) {
  return $arg1;
}

function generic_shape<T>((T, T) $arg1): (T, T) {
  return $arg1;
}

type TaggedCoordinate =
  shape(?'tag' => string, 'coord' => shape('x' => float, 'y' => float));
