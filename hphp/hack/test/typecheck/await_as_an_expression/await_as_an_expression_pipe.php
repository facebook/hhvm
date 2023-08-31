<?hh
<<file: __EnableUnstableFeatures('pipe_await')>>

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<void> {
  await f() |> await f();
}
