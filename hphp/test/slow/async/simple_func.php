<?hh

async function ret1() { return 1; }
async function await1() {
  $b = await ret1();
  return 1 + $b;
}

var_dump(ret1()->join());
var_dump(await1()->join());
