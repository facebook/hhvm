<?hh

function correct(): noreturn {
  throw new Exception('not returning here');
}

async function meh(): Awaitable<int> {
  return 4;
}

async function generic(): Awaitable<noreturn> {
  await meh();
  throw new Exception('not returning here');
}
