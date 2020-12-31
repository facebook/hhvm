<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
async function toasync(int $a)[rx]: Awaitable<int> {
  return $a;
}

<<__Rx>>
async function f()[rx]: Awaitable<void> {
  // OK
  await (1 |> toasync($$));
}
