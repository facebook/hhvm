<?hh

function expr(supportdyn<nonnull> $sd): void {
  $sd === 1;
}

function stmt(supportdyn<nonnull> $sd): void {
  if ($sd) {}
  switch ($sd) {
    case 42:
      break;
  }
}
