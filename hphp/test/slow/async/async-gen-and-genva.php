<?hh

async function block() {
  await RescheduleWaitHandle::create(0, 0);
}

async function baz(int $num) {
  if ($num % 2 == 1) {
    await block();
  }

  return $num;
}

async function bar() {
  $exp = 0;
  foreach (varray[0, 1] as $block_dep1) {
    foreach (varray[0, 1] as $block_dep2) {
      foreach (varray[0, 1] as $block_outer) {
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

async function foo() {
  foreach (bar() await as list($x, $y)) {
    concurrent {
      $a = await baz($x);
      $b = await baz($y);
    }
    yield 100 * $a + $b;
  }
}

async function main() {
  foreach (foo() await as $val) {
    var_dump($val);
  }
}


<<__EntryPoint>>
function main_async_gen_and_genva() {
\HH\Asio\join(main());
}
