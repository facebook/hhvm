<?hh // strict

function correct(): noreturn {
  throw new Exception('not returning here');
}

async function meh(): Awaitable<int> {
  // UNSAFE
}

async function generic(): Awaitable<noreturn> {
  await meh();
  throw new Exception('not returning here');
}
