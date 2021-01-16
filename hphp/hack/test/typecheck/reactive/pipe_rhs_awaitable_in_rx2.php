<?hh
<<__Rx>>
async function toasync(int $a)[rx]: Awaitable<int> {
  return $a;
}

<<__Rx>>
async function f()[rx]: Awaitable<void> {
  // ERROR
  1 |> toasync($$);
}
