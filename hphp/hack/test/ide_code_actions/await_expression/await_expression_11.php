<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

function example(): void {
  // The refactor should change the lambda to an async that returns `Awaitable<int>`
  (): int ==> {
    $_ = /*range-start*/ gen_int()/*range-end*/ + 3;
  };
}
