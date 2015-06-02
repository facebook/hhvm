<?hh

class F {
  async function ret1() { return 1; }
  async function await1() {
    $b = await F::ret1();
    return 1 + $b;
  }
}

var_dump(HH\Asio\join(F::ret1()));
var_dump(HH\Asio\join(F::await1()));
