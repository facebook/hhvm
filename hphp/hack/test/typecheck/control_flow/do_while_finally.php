<?hh

function f(): void {
  do {
    $a = 1;
    try {
    } finally {
      $a;
    }
  } while (1 == 2);
}
