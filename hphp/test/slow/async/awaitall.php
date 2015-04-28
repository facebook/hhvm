<?hh

async function answer() {
  await reschedule();
  return 42;
};
function reschedule() {
  return RescheduleWaitHandle::create(1, 1);
}

function t(WaitHandle $wh, $a): void {
  echo $wh->getName(), ' ', count($a), "\nbefore: ";
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

  // single element
  $vectors[] = Vector {reschedule()};
  $maps[] = Map {0 => reschedule()};
  $arrays[] = array(0 => reschedule());

  $arrays[] = array(reschedule());

  // empty
  $arrays[] = array();
  $vectors[] = Vector {};
  $maps[] = Map {};
  $vectors[] = ImmVector {};
  $maps[] = ImmMap {};

  // multiple elements
  $vectors[] = Vector {reschedule(), answer()};
  $maps[] = Map {0 => reschedule(), 1 => answer() };
  $arrays[] = array(0 => reschedule(), 1 => answer() );
  $arrays[] = array(0 => reschedule(), 2 => answer() );
  $arrays[] = array(0 => reschedule(), 'zero' => answer() );
  $arrays[] = array(reschedule(), answer());
  $arrays[] = array(0 => reschedule(), 1 => answer());
  $arrays[] = array('zero' => reschedule(), 'one' => answer());

  $a = array(0 => reschedule());
  $a[2] = answer();
  $arrays[] = $a;

  $arrays[] = array_map(
    $x ==> $x%2 ? reschedule() : answer(),
    range(0, 30),
  );

  return tuple($vectors, $maps, $arrays);
}

echo "children only\n";
list($vectors, $maps, $arrays) = get_handles();

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

echo "parents\n";
function get_wrapped_handles() {
  list($vectors, $maps, $arrays) = $handles = get_handles();
  return tuple(
    $vectors->map($v ==> AwaitAllWaitHandle::fromVector($v)),
    $maps->map($m ==> AwaitAllWaitHandle::fromMap($m)),
    $arrays->map($a ==> AwaitAllWaitHandle::fromArray($a)),
  );
}

list($vectors, $maps, $arrays) = $handles = get_wrapped_handles();
$wh = AwaitAllWaitHandle::fromVector($vectors);
t($wh, $vectors);

$wh = AwaitAllWaitHandle::fromMap(new ImmMap($maps));
t($wh, $maps);

$wh = AwaitAllWaitHandle::fromArray($arrays->toArray());
t($wh, $arrays);


list($vectors, $maps, $arrays) = $handles = get_wrapped_handles();
echo "grandchildren\n";
$top = array(
  AwaitAllWaitHandle::fromVector($vectors),
  AwaitAllWaitHandle::fromVector($maps),
  AwaitAllWaitHandle::fromVector($arrays),
);
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
