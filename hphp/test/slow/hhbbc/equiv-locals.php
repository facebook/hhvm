<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function loop() :mixed{
  $a = $b = $c = $d = false;
  while (true) {
    if ($a) continue;
    if ($c) {
      echo "Breaking out of loop...\n";
      break;
    }
    if ($d) {
      echo "Shouldn't reach me!\n";
      break;
    }
    $b = (bool)__hhvm_intrinsics\launder_value(true);
    $c = (bool)__hhvm_intrinsics\launder_value(true);
    $d = (bool)__hhvm_intrinsics\launder_value(true);
  }
}

<<__EntryPoint>>
function main_equiv_locals() :mixed{
loop();
}
