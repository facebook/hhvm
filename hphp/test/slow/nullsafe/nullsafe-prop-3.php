<?hh // strict


<<__EntryPoint>>
function main_nullsafe_prop_3() {
$x = null;
if (false) {
  $x?->y += 1; // parse error
}
}
