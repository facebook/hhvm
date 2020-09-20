<?hh // partial

<<__Rx>>
function f(): void {
}

<<__Rx>>
async function g(): Awaitable<void> {
  // OK
  await async {
    f();
  };
}
