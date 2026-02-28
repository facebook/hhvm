<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(0, 0);
}

async function baz(int $num) :Awaitable<mixed>{
  if ($num % 2 == 1) {
    await block();
  }

  return $num;
}

async function bar() :AsyncGenerator<mixed,mixed,void>{
  $exp = 0;
  foreach (vec[0, 1] as $block_dep1) {
    foreach (vec[0, 1] as $block_dep2) {
      foreach (vec[0, 1] as $block_outer) {
        if ($block_outer) {
          await block();
        }

        yield tuple(
          $exp * 10 + $block_dep1,
          $block_outer * 10 + $block_dep2,
        );
        $exp++;
      }
    }
  }
}

async function foo() :AsyncGenerator<mixed,mixed,void>{
  foreach (bar() await as list($x, $y)) {
    concurrent {
      $a = await baz($x);
      $b = await baz($y);
    }
    yield 100 * $a + $b;
  }
}

async function main() :Awaitable<mixed>{
  foreach (foo() await as $val) {
    var_dump($val);
  }
}


<<__EntryPoint>>
function main_async_gen_and_genva() :mixed{
\HH\Asio\join(main());
}
