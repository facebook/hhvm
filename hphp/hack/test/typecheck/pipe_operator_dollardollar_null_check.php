<?hh

interface I {
  public function get(): int;
}

function foo(?I $x): int {
  return $x |> $$ === null ? 5 : $$->get();
}
