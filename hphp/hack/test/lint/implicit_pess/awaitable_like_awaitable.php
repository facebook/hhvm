<?hh

interface I {}

function foo(I $i): void {
  async () ==> gen($i);
}

async function gen(I $vc): Awaitable<void> {}
