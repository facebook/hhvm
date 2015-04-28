<?hh

try {
  $wh = stream_await(fopen('php://memory', 'rw'),
    STREAM_AWAIT_READ | STREAM_AWAIT_WRITE, 0.0);
  var_dump(HH\Asio\join($wh));
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
