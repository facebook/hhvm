<?hh

// Testing function
async function right_hint(): ?Awaitable<int> {
  throw new Exception();
}
