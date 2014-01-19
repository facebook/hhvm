<?hh

class F {
  async function ret1() { return 1; }
  async function await1() {
    $b = await F::ret1();
    return 1 + $b;
  }
}

var_dump(F::ret1()->join());
var_dump(F::await1()->join());
