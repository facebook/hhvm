<?hh // strict

/* Invalid construction: shape field names may not start with numbers */

function test(): void {
  $x = shape('123abc' => 1, 'field2' => true);
}
