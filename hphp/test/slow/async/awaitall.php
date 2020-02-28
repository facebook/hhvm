<?hh

async function answer() {
  await reschedule();
  return 42;
}function reschedule() {
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
  $maps = Vector {};
  $vectors = Vector {};
  $arrays = Vector {};
  $vecs = Vector {};
  $dicts = Vector {};

  // single element
  $vectors[] = Vector {reschedule()};
  $maps[] = Map {0 => reschedule()};
  $arrays[] = darray[0 => reschedule()];
  $vecs[] = vec[reschedule()];
  $dicts[] = dict[0 => reschedule()];

  $arrays[] = varray[reschedule()];

  // empty
  $arrays[] = varray[];
  $vectors[] = Vector {};
  $maps[] = Map {};
  $vecs[] = vec[];
  $dicts[] = dict[];
  $vectors[] = ImmVector {};
  $maps[] = ImmMap {};

  // multiple elements
  $vectors[] = Vector {reschedule(), answer()};
  $maps[] = Map {0 => reschedule(), 1 => answer() };
  $arrays[] = darray[0 => reschedule(), 1 => answer() ];
  $arrays[] = darray[0 => reschedule(), 2 => answer() ];
  $arrays[] = darray[0 => reschedule(), 'zero' => answer() ];
  $arrays[] = varray[reschedule(), answer()];
  $arrays[] = darray[0 => reschedule(), 1 => answer()];
  $arrays[] = darray['zero' => reschedule(), 'one' => answer()];
  $vecs[] = vec[reschedule(), answer()];
  $dicts[] = dict[0 => reschedule(), 1 => answer()];
  $dicts[] = dict[0 => reschedule(), 2 => answer()];
  $dicts[] = dict[0 => reschedule(), 'zero' => answer()];
  $dicts[] = dict['zero' => reschedule(), 'one' => answer()];

  $a = darray[0 => reschedule()];
  $a[2] = answer();
  $arrays[] = $a;

  $d = dict[0 => reschedule()];
  $d[2] = answer();
  $dicts[] = $d;

  $arrays[] = array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  );
  $vecs[] = vec(array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  ));
  $dicts[] = dict(array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  ));

  return tuple($vectors, $maps, $arrays, $vecs, $dicts);
}
function get_wrapped_handles() {
  list($vectors, $maps, $arrays, $vecs, $dicts) = $handles = get_handles();
  return tuple(
    $vectors->map($v ==> AwaitAllWaitHandle::fromVector($v)),
    $maps->map($m ==> AwaitAllWaitHandle::fromMap($m)),
    $arrays->map($a ==> AwaitAllWaitHandle::fromArray($a)),
    $vecs->map($v ==> AwaitAllWaitHandle::fromVec($v)),
    $dicts->map($d ==> AwaitAllWaitHandle::fromDict($d))
  );
}

<<__EntryPoint>>
function main_awaitall() {
;

echo "children only\n";
list($vectors, $maps, $arrays, $vecs, $dicts) = get_handles();

foreach ($vectors as $v) {
  $wh = AwaitAllWaitHandle::fromVector($v);
  t($wh, $v);
}
foreach ($maps as $m) {
  $wh = AwaitAllWaitHandle::fromMap($m);
  t($wh, $m);
}
foreach ($arrays as $a) {
  $wh = AwaitAllWaitHandle::fromArray($a);
  t($wh, $a);
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

list($vectors, $maps, $arrays, $vecs, $dicts) = $handles = get_wrapped_handles();
$wh = AwaitAllWaitHandle::fromVector($vectors);
t($wh, $vectors);

$wh = AwaitAllWaitHandle::fromMap(new ImmMap($maps));
t($wh, $maps);

$wh = AwaitAllWaitHandle::fromArray($arrays->toArray());
t($wh, $arrays);

$wh = AwaitAllWaitHandle::fromVec(vec($vecs));
t($wh, $vecs);

$wh = AwaitAllWaitHandle::fromDict(dict($dicts));
t($wh, $dicts);

list($vectors, $maps, $arrays, $vecs, $dicts) = $handles = get_wrapped_handles();
echo "grandchildren\n";
$top = varray[
  AwaitAllWaitHandle::fromVector($vectors),
  AwaitAllWaitHandle::fromVector($maps),
  AwaitAllWaitHandle::fromVector($arrays),
  AwaitAllWaitHandle::fromVector($vecs),
  AwaitAllWaitHandle::fromVector($dicts)
];
$wh = AwaitAllWaitHandle::fromArray($top);
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
