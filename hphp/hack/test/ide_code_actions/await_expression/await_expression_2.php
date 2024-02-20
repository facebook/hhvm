<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}


async function repro(): Awaitable<mixed> {
  return /*range-start*/gen_int()/*range-end*/ + 3;
}
