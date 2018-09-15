<?hh // experimental

function string_predicate(string $s): bool {
  return false;
}

function foo(): void {
  let x = 10;
  do {
    let x = "ten";
  } while (string_predicate(x));
}
