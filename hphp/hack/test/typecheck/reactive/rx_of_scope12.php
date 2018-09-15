<?hh

function f(): void {
}

<<__Rx>>
async function g(): Awaitable<void> {
  // ERROR
  await async {
    f();
  };
}
