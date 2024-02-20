<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

function example(): void {
  // The refactor should not chang ethe return type of this lambda:
  (): Awaitable<mixed> ==> {
    $_ = /*range-start*/ gen_int()/*range-end*/ + 3;
  };
}
