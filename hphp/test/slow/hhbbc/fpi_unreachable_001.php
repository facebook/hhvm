<?hh

function blah() {
  return darray['x' => 2];
}

abstract class X {
  async function await_always_throw() {
    return await $this->foo();
  }

  async function foo() {
    $array = blah(static::heh() ? 2 : 3, 2);
    // unreachable:
    for ($i = 0; $i < 10; ++$i) echo "hi\n";
    return varray[$array['x']];
  }

  abstract protected static function heh(): varray<string>;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
