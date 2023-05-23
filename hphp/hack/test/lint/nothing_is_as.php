<?hh

function nothing_is_as(nothing $n): void {
  $n as int;
  if ($n is int) { echo 'hi'; }
}
