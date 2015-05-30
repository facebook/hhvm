<?hh

type Point = shape(
  'x' => int,
  'y' => int,
);

type Recur = shape(
  'shape' => Recur
);

type Point3 = shape(
  'x' => int,
  'y' => int,
  'z' => int,
);

type Optional = shape(
  ?'optional_field' => ?int,
);

function foo(Point $x) : Point {
  $x['x'] = 42;
  return $x;
}

function foo2() : shape('x' => int, 'y' => int) {
  $x = shape(
    'x' => 12,
    'y' => 12,
  );
  return $x;
}

function foo3<T as shape('x' => int)>(T $pt) : T {
  $pt['x'] = 42;
  return $pt;
}

function foo31<T as shape('x' => int, 'y' => string)>(T $pt) : T {
  $pt['x'] = 42;
  return $pt;
}

function foo32<T as shape('x' => int, 'y' => string,)>(T $pt) : T {
  $pt['x'] = 42;
  return $pt;
}

function foo4() {
  $x = shape(
    'x' => 12,
    'y' => 12
  );
}

function foo5<T as Point>(T $pt) : T {
  $pt['x'] = 42;
  return $pt;
}

function foo6<T as shape()>(T $pt) : T {
  return $pt;
}

function foo7(shape('id' => int, 'name' => string) $x) : int {
  return 12;
}

type Foobar = shape(
  'foobar' => string,
);

function foo8(string $foo, string $bar) : Foobar {
  $x = shape(
    'foobar' => $foo . $bar
  );
  return 12;
}

// You can have a shape as a parameter to a user attribute.
<<
  AttrTakingShape(
    shape(
      'x' => 1,
      'y' => 1,
    )
  )
>>
class SomeClass {}

// These parse, but don't work at runtime currently:
const Foobar SCALAR_SHAPE = shape(
  'foobar' => 'constant'
);
const Point3 SCALAR_SHAPE2 = shape(
  'x' => 12,
  'y' => 13,
  'z' => 11,
);
const SCALAR_SHAPE3 = shape(
  'x' => 12,
  'foo' => 1.0
);
