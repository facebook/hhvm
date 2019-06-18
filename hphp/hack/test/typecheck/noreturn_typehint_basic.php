<?hh // strict

function correct(): noreturn {
  throw new Exception('not returning here');
}

/* HH_FIXME[4110] */
async function meh(): Awaitable<int> {
}

async function generic(): Awaitable<noreturn> {
  await meh();
  throw new Exception('not returning here');
}
