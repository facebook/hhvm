<?hh // partial

<<__Rx>>
async function toasync(int $a): Awaitable<int> {
  return $a;
}

<<__Rx>>
async function f(): Awaitable<void> {
  // ERROR
  1 |> toasync($$);
}
