<?hh // strict

/* Invalid construction: shape field names may not be int-like strings */
function test(): void {
  $x = shape('123_456_789' => 1, 'field2' => true);
}
