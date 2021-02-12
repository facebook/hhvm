<?hh

function f(): void {
}


async function g(): Awaitable<void> {
  // OK
  await async {
    f();
  };
}
