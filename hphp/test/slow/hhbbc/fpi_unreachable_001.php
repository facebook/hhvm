<?hh

function blah() {
  return array('x' => 2);
}

abstract class X {
  async function await_always_throw() {
    return await $this->foo();
  }

  async function foo() {
    $array = blah(static::heh() ? 2 : 3, 2);
    // unreachable:
    for ($i = 0; $i < 10; ++$i) echo "hi\n";
    return array($array['x']);
  }

  abstract protected static function heh(): array<string>;
}
