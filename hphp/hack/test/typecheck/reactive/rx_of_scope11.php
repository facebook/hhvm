<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

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
