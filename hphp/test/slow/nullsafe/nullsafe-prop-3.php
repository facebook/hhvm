<?hh


<<__EntryPoint>>
function main_nullsafe_prop_3() :mixed{
$x = null;
if (false) {
  $x?->y += 1; // parse error
}
}
