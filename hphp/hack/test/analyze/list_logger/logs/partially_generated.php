<?hh
// @partially-generated SignedSource<<deadbeef>>

function generated_before(): void {
  $t = tuple(1, "hello");
  list($a, $b) = $t;
}

/* BEGIN MANUAL SECTION my_manual_section */

function manual_section(): void {
  $t = tuple(3, "world");
  list($c, $d) = $t;
}

/* END MANUAL SECTION */

function generated_after(): void {
  $t = tuple(5, "bye");
  list($e, $f) = $t;
}
