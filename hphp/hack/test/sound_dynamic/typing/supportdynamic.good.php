<?hh

function expr(supportdynamic $sd): void {
  $sd === 1;
}

function stmt(supportdynamic $sd): void {
  if ($sd) {}
  switch ($sd) {
    case 42:
      break;
  }
}
