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
  foreach (array(0, 1) as $block_dep1) {
    foreach (array(0, 1) as $block_dep2) {
      foreach (array(0, 1) as $block_outer) {
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
    list($a, $b) = await genva(
      baz($x),
      baz($y),
    );
    yield 100 * $a + $b;
  }
}

async function main() {
  foreach (foo() await as $val) {
    var_dump($val);
  }
}

\HH\Asio\join(main());
