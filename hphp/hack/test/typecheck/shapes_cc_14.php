<?hh // strict

/*
 * Invalid declaration: shape key names may not start with numbers
 */

type myshape = shape(
  '123abc' => int,
  'field2' => bool,
);
