<?hh

const EXPECTED = vec[1, 2, 3];

function nice_type(mixed $x): string {
  $t = gettype($x);
  if ($t === 'object') return get_class($x);
  return $t;
}

async function blockme(int $n): Awaitable<int> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  return $n;
}

function get_good_cases(): vec<Container<Awaitable<int>>> {
  $base = vec[blockme(1), blockme(2), blockme(3)];
  return vec[
    $base,
    dict($base),
    darray($base),
    varray($base),
    new Vector($base),
    new Map($base),
  ];
}

async function await_all(
  Container<Awaitable<mixed>> $c
): Awaitable<Container<mixed>> {
  await AwaitAllWaitHandle::fromContainer($c);
  foreach ($c as $k => $v) {
    $c[$k] = \HH\Asio\join($v);
  }
  return $c;
}

async function good_tests(): Awaitable<void> {
  foreach (get_good_cases() as $case) {
    $result = vec(await await_all($case));
    invariant(
      $result === EXPECTED,
      'Got unexpected result: %s',
      print_r($result, true),
    );
    printf("Passed for: %s\n", nice_type($case));
  }
}

async function bad_tests(): Awaitable<void> {
  $cases = vec[
    Set{},
    ImmSet{},
    Pair{0, 1},
    keyset[],
    new stdClass(),
    good_tests<>,
  ];
  foreach ($cases as $thing) {
    printf('For type %s', nice_type($thing));
    try {
      await AwaitAllWaitHandle::fromContainer($thing);
      printf(" FAILED to throw\n");
    } catch (Exception $e) {
      printf(" threw %s\n", $e->getMessage());
    }
  }
}

<<__EntryPoint>> async function main(): Awaitable<void> {
  await good_tests();
  await bad_tests();
}
