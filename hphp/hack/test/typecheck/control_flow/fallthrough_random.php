<?hh // strict

function f(): void {
  if (true) {
    $x = '';
    // Randomly placed FALLTHROUGH
    // FALLTHROUGH
  }
  $x = 0;
  switch ('') {
    case 'value':
      expect_int($x);
      break;

    default:
      break;
  }
}

function expect_int(int $x): void {}
