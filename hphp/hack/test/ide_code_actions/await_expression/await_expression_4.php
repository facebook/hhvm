<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}


async function repro(): Awaitable<mixed> {
  // We should offer the refactoring.
  return await () ==> /*range-start*/gen_int()/*range-end*/ + 3;
}
