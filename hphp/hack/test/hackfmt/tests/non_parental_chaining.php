<?hh

$foo->property->otherProperty->method(
  $a_____________________,
  $b_____________________,
  $c_____________________,
);

$foo->property->method()->otherMethod(
  $a_____________________,
  $b_____________________,
  $c_____________________,
);

// This example is not preserved because we can fit the otherMethod call on one
// line. That solution is considered more desirable because of the cost of the
// Span wrapping otherMethod's argument list.
$foo->property->method()->otherMethod(
  $a_____________________,
  $b_____________________,
);
