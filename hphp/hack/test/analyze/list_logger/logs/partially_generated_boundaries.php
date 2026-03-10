<?hh
// @partially-generated SignedSource<<deadbeef>>

// Generated: immediately before the BEGIN marker
function generated_just_before(): void {
  list($a, $b) = tuple(1, "a");
}
/* BEGIN MANUAL SECTION first_section */
function manual_at_start(): void {
  list($c, $d) = tuple(2, "b");
}

function manual_at_end(): void {
  list($e, $f) = tuple(3, "c");
}
/* END MANUAL SECTION */
function generated_just_after(): void {
  list($g, $h) = tuple(4, "d");
}

/* BEGIN MANUAL SECTION second_section */
function manual_second(): void {
  list($i, $j) = tuple(5, "e");
}
/* END MANUAL SECTION */

function generated_at_end(): void {
  list($k, $l) = tuple(6, "f");
}
