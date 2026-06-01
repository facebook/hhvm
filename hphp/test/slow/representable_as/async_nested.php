<?hh

// Awaitable<RepresentableAs<int>> should enforce 'int' at runtime
async function returns_bad_async(): Awaitable<HH\Runtime\RepresentableAs<int>> {
  return HH\FIXME\UNSAFE_CAST<string, HH\Runtime\RepresentableAs<int>>("hello");
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await returns_bad_async();
}
