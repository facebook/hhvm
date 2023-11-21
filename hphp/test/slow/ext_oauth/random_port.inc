<?hh

function get_random_port() {
  return rand(10000, 65000);
}

function random_free_port() {
  $error = error_reporting();
  error_reporting(0);
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    if ($socket = socket_create_listen($port)) {
      socket_close($socket);
      return $port;
    }
  }
  error_reporting($error);
  return 0;
}
