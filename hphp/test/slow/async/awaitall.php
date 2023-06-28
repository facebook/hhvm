<?hh

async function answer() :Awaitable<mixed>{
  await reschedule();
  return 42;
}

function reschedule() :mixed{
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


function get_handles() :mixed{
  $vecs = Vector {};
  $dicts = Vector {};

  // single element
  $vecs[] = vec[reschedule()];
  $dicts[] = dict[0 => reschedule()];

  // empty
  $vecs[] = vec[];
  $dicts[] = dict[];

  // multiple elements
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

  return tuple($vecs, $dicts);
}

function get_wrapped_handles() :mixed{
  list($vecs, $dicts) = get_handles();
  return tuple(
    $vecs->map($v ==> AwaitAllWaitHandle::fromVec($v)),
    $dicts->map($d ==> AwaitAllWaitHandle::fromDict($d))
  );
}

<<__EntryPoint>>
function main_awaitall() :mixed{
  echo "children only\n";
  list($vecs, $dicts) = get_handles();

  foreach ($vecs as $v) {
    $wh = AwaitAllWaitHandle::fromVec($v);
    t($wh, $v);
  }
  foreach ($dicts as $d) {
    $wh = AwaitAllWaitHandle::fromDict($d);
   t($wh, $d);
  }

  echo "parents\n";
  list($vecs, $dicts) = get_wrapped_handles();

  $wh = AwaitAllWaitHandle::fromVec(vec($vecs));
  t($wh, $vecs);

  $wh = AwaitAllWaitHandle::fromDict(dict($dicts));
  t($wh, $dicts);

  echo "grandchildren\n";
  list($vecs, $dicts) = $handles = get_wrapped_handles();

  $top = vec[
    AwaitAllWaitHandle::fromVec(vec($vecs)),
    AwaitAllWaitHandle::fromVec(vec($dicts))
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
