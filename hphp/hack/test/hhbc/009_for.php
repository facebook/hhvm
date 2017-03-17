<?hh // strinc

function test(): void {
  $x = 42;
  for ($i = 0; $i < $x; $i += 1) {
    echo $i;
  }
}
