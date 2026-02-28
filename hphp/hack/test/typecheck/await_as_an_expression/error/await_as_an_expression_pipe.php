<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<void> {
  await f() |> await f();
}
