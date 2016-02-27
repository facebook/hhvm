<?hh

function foo(): int {
  return 1;
}

function bar(): int {
  return 2;
}

function piped(): int {
  return foo() |> bar();
}
