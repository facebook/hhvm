<?hh // strict

/*
 * Invalid declaration: shape key names may not be empty.
 */

type myshape = shape(
  '' => int,
  'field2' => bool,
);
