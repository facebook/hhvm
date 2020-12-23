<?hh

function purification_fail(
  (function ()[local]: void) $f
): (function ()[]: void) {
  return $f;
}
