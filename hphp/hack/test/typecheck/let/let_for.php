<?hh // experimental

function foo(int $count): void {
  for ($i = 0; $i < $count; $i++) {
    let twice : int = $i * 2;
  }
  echo twice; // error
}
