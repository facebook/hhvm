<?hh

// Testing function
async function wrong_hint(): array<int> {
  throw new Exception();
}
