<?hh

function takes_int(int $_): void {}

function main(num $n, arraykey $a): void {
  if ($n === $a) {
    takes_int($n); // No error, int is the only thing it can be
    takes_int($a); // No error, int is the only thing it can be
  }
  takes_int($n); // Error
  takes_int($a); // Error
}
