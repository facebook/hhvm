<?hh

function test_switch_with_default(int $x)[rx]: int {
  switch ($x) {
    case 0: return 5;
    case 1: return 6;
    default: return -1;
  }
}

function test_switch_no_default(int $x)[rx]: int {
  try {
    switch ($x) {
      case 0: return 5;
      case 1: return 6;
    }
  } catch (Exception $_) {}
  return -1;
}

<<__EntryPoint>>
function main() {
  for ($i = 0; $i < 3; $i++) {
    var_dump(test_switch_with_default($i));
    var_dump(test_switch_no_default($i));
  }
}
