<?hh // strict

type Point2D = shape('x' => int, 'y' => int);

function cast_shape(Point2D $p): nonnull {
  return $p;
}
