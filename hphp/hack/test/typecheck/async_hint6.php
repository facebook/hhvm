<?hh

// Testing function
async function right_hint(): Awaitable<int> {
  throw new Exception();
}

// Testing function
async function no_hint() {
  throw new Exception();
}
