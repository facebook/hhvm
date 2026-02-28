<?hh

function bar($a) :mixed{
  var_dump(__METHOD__);
  fb_enable_code_coverage();
}

async function gen($a) :Awaitable<mixed>{
  error_log('In gen');
  array_map(bar<>, $a);
  error_log('Finished in gen');
  if ($a[0]) {
    await RescheduleWaitHandle::create(0, 0); // simulate blocking I/O
  }
  return $a;
}

function main($a) :mixed{
  $result = HH\Asio\join(gen($a));
  var_dump($result);
}
<<__EntryPoint>> function main_entry(): void {
main(vec[42]);
}
