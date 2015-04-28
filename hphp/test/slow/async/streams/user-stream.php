<?hh

class MyStream {
  public function stream_open($path, $mode, $options, &$opened_path) {
    return true;
  }
}
stream_wrapper_register('mystream', 'MyStream');

try {
  $wh = stream_await(fopen('mystream://foo', 'rw'),
    STREAM_AWAIT_READ | STREAM_AWAIT_WRITE, 0.0);
  var_dump(HH\Asio\join($wh));
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
