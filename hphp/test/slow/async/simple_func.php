<?hh

async function ret1() { return 1; }
async function await1() {
  $b = await ret1();
  return 1 + $b;
}

var_dump(HH\Asio\join(ret1()));
var_dump(HH\Asio\join(await1()));
