<?hh

function takes_int(int $_): void {}

function main(?int $i): void {
  if ($i !== 42 || $i !== 43) {
    takes_int($i);
    return;
  }

  // If it wasn't an int, we would have returned
  takes_int($i);
}
