<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

function example(): Awaitable<mixed> {
  // no `asnyc` should be added to this lambda
  return () ==> {
    return async () ==> {
      $_ = /*range-start*/gen_int()/*range-end*/ + 3;
    }
  }
}
