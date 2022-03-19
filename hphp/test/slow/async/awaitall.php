<?hh

async function answer() {
  await reschedule();
  return 42;
}

function reschedule() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

function t(Awaitable $wh, $a): void {
  echo HH\Asio\name($wh), ' ', count($a), "\nbefore: ";
  foreach ($a as $k => $aa) {
    echo "$k:", HH\Asio\has_finished($aa) ? 1 : 0, ",";
  }
  echo "\nafter : ";
  HH\Asio\join($wh);
  foreach ($a as $k => $aa) {
    echo "$k:", HH\Asio\has_finished($aa) ? 1 : 0, ",";
  }
  echo "\n";
}


function get_handles() {
  $vectors = Vector {};
  $maps = Vector {};
  $vecs = Vector {};
  $dicts = Vector {};

  // single element
  $vectors[] = Vector {reschedule()};
  $maps[] = Map {0 => reschedule()};
  $vecs[] = vec[reschedule()];
  $dicts[] = dict[0 => reschedule()];

  // empty
  $vectors[] = Vector {};
  $maps[] = Map {};
  $vecs[] = vec[];
  $dicts[] = dict[];
  $vectors[] = ImmVector {};
  $maps[] = ImmMap {};

  // multiple elements
  $vectors[] = Vector {reschedule(), answer()};
  $maps[] = Map {0 => reschedule(), 1 => answer() };
  $vecs[] = vec[reschedule(), answer()];
  $dicts[] = dict[0 => reschedule(), 1 => answer()];
  $dicts[] = dict[0 => reschedule(), 2 => answer()];
  $dicts[] = dict[0 => reschedule(), 'zero' => answer()];
  $dicts[] = dict['zero' => reschedule(), 'one' => answer()];

  $d = dict[0 => reschedule()];
  $d[2] = answer();
  $dicts[] = $d;

  $vecs[] = vec(array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  ));
  $dicts[] = dict(array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  ));

  return tuple($vectors, $maps, $vecs, $dicts);
}

function get_wrapped_handles() {
  list($vectors, $maps, $vecs, $dicts) = get_handles();
  return tuple(
    $vectors->map($v ==> AwaitAllWaitHandle::fromVector($v)),
    $maps->map($m ==> AwaitAllWaitHandle::fromMap($m)),
    $vecs->map($v ==> AwaitAllWaitHandle::fromVec($v)),
    $dicts->map($d ==> AwaitAllWaitHandle::fromDict($d))
  );
}

<<__EntryPoint>>
function main_awaitall() {
  echo "children only\n";
  list($vectors, $maps, $vecs, $dicts) = get_handles();

  foreach ($vectors as $v) {
    $wh = AwaitAllWaitHandle::fromVector($v);
    t($wh, $v);
  }
  foreach ($maps as $m) {
    $wh = AwaitAllWaitHandle::fromMap($m);
    t($wh, $m);
  }
  foreach ($vecs as $v) {
    $wh = AwaitAllWaitHandle::fromVec($v);
    t($wh, $v);
  }
  foreach ($dicts as $d) {
    $wh = AwaitAllWaitHandle::fromDict($d);
   t($wh, $d);
  }

  echo "parents\n";
  list($vectors, $maps, $vecs, $dicts) = get_wrapped_handles();

  $wh = AwaitAllWaitHandle::fromVector($vectors);
  t($wh, $vectors);

  $wh = AwaitAllWaitHandle::fromMap(new ImmMap($maps));
  t($wh, $maps);

  $wh = AwaitAllWaitHandle::fromVec(vec($vecs));
  t($wh, $vecs);

  $wh = AwaitAllWaitHandle::fromDict(dict($dicts));
  t($wh, $dicts);

  echo "grandchildren\n";
  list($vectors, $maps, $vecs, $dicts) = $handles = get_wrapped_handles();

  $top = vec[
    AwaitAllWaitHandle::fromVector($vectors),
    AwaitAllWaitHandle::fromVector($maps),
    AwaitAllWaitHandle::fromVector($vecs),
    AwaitAllWaitHandle::fromVector($dicts)
  ];
  $wh = AwaitAllWaitHandle::fromVec($top);

  $finished = () ==> {
    foreach ($handles as $hs) {
      echo count($hs), "\t";
      foreach ($hs as $h) {
        echo HH\Asio\has_finished($h) ? 1 : 0;
      }
      echo "\n";
    }
  };

  $finished();
  HH\Asio\join($wh);
  $finished();

  echo "done\n";
}
