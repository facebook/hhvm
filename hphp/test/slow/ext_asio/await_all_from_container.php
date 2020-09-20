<?hh

const EXPECTED = vec[1, 2, 3];

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

<<__EntryPoint>>
async function good_tests(): Awaitable<void> {
  foreach (get_good_cases() as $case) {
    $result = vec(await await_all($case));
    invariant(
      $result === EXPECTED,
      'Got unexpected result: %s',
      print_r($result, true),
    );
    printf("Passed for: %s\n", gettype($case));
  }
}
