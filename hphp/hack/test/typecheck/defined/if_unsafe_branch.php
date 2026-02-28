<?hh

function f(): void {
    if (true) {
      $x = 0;
    } else {
      // UNSAFE_BLOCK
      $x = 1;
    }

    echo $x;
}
