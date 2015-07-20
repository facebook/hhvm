<?hh //strict

function do_switch_1(int $x): void {
  switch ($x+0) {
  default:
    echo "default\n";
    break;
  case 0:
    echo "i equals 0\n";
    // FALLTHROUGH
  case 1:
    echo "i equals 1 (or whatever)\n";
    break;
  case 2:
    echo "i equals 2\n";
    break;
  }
  echo "done\n";
}

function test(): void {
  do_switch_1(0);
  do_switch_1(1);
  do_switch_1(2);
  do_switch_1(3);
}
