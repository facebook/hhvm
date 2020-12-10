<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function f(): void {
}

<<__Rx>>
async function g(): Awaitable<void> {
  // ERROR
  await async {
    f();
  };
}
