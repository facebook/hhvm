<?hh

function blah() :mixed{
  return dict['x' => 2];
}

abstract class X {
  async function await_always_throw() :Awaitable<mixed>{
    return await $this->foo();
  }

  async function foo() :Awaitable<mixed>{
    $array = blah(static::heh() ? 2 : 3, 2);
    // unreachable:
    for ($i = 0; $i < 10; ++$i) echo "hi\n";
    return vec[$array['x']];
  }

  abstract protected static function heh(): varray<string>;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
