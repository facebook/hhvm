<?hh // partial

<<__Rx>>
async function toasync(int $a): Awaitable<int> {
  return $a;
}

<<__Rx>>
async function f(): Awaitable<void> {
  // OK
  await genva(1 |> toasync($$));
}
