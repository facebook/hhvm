<?hh

/*
 * Invalid declaration: shape field names may not be int-like strings
 */

type myshape = shape(
  '123' => int,
  'field2' => bool
);
