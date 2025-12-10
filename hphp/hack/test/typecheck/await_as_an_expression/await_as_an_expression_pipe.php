<?hh
<<file: __EnableUnstableFeatures('pipe_await')>>

async function f(): Awaitable<?int> {
  return 1;
}

async function g(): Awaitable<void> {
  await f() |> await f();
  // await is not allowed in the RHS of a null-safe pipe since RHS is conditionally executed
  await f() |?> await f();
}
