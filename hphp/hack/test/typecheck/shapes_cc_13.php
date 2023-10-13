<?hh

/* Invalid construction: shape field names may not be empty */

function test(): void {
  $x = shape('' => 1, 'field2' => true);
}
