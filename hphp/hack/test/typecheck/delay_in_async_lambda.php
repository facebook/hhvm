<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 1;
}

function foo(): void {
  $f = async () ==> {
    $_ = delay bar();
  };
}
