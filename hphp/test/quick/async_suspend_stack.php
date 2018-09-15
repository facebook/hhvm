<?hh

function bar($a, $k) {
  var_dump(__METHOD__);
  @fb_enable_code_coverage();
}

async function gen($a) {
  error_log('In gen');
  array_walk(&$a, 'bar');
  error_log('Finished in gen');
  if ($a[0]) {
    await RescheduleWaitHandle::Create(0, 0); // simulate blocking I/O
  }
  return $a;
}

function main($a) {
  $result = HH\Asio\join(gen($a));
  var_dump($result);
}

main(array(42));
