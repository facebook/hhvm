<?hh

// Keys must not be numeric or start with integers.
type BadPoint = shape(
  '123' => int,
  '124' => int,
);


