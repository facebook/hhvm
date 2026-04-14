<?hh

// async function with unenforced inner type
async function foo<T>(): Awaitable<T> {
  return HH\FIXME\UNSAFE_CAST<mixed, T>(null);
//       ^ enforcement-at-caret
}
