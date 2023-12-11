<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main(bool $b1, bool $b2) :mixed{
  $c = $b1 ? 1.0 : 100;
  if ($b2) $c = 10000;
  return 1 / ((float)$c);
}


<<__EntryPoint>>
function main_select_int_dbl() :mixed{
foreach (vec[false, true, false, true] as $b1) {
  foreach (vec[false, true, false, true] as $b2) {
    var_dump(main($b1, $b2));
  }
}
}
