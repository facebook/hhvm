<?hh

function purification_fail(
  (function ()[write_props]: void) $f
): (function ()[]: void) {
  return $f;
}
