<?hh

abstract class Y {
  abstract public function g();
}

class Z extends Y {
  public async function x() { return false; }
  public function g() { return $this->x(); }
}

class X {
  public static async function f(
    ?Y $group,
  ): Awaitable<bool> {
    $is_member = await $group?->g();
    return $is_member ?? false;
  }
}

<<__EntryPoint>>
async function main() {
  $y = new Z();
  for ($i = 0; $i < 10; $i++) {
    await X::f($y);
  }
  $x = await X::f($y);
  var_dump($x);
}
