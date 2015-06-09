<?hh //strict

/* Shape field names cannot be strings enclosed by double quotes. */

function test(): void {
  $x = shape("field1" => 12); // parse error
}
