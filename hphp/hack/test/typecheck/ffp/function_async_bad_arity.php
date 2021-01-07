<?hh

// This should be a parse error, because we don't know what runtime
// type enforcement to do.
async function foo(): Awaitable<int, int> {
  throw new Exception();
}
