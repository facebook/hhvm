<?hh


<<__EntryPoint>>
function main_ssl_socket_null_byte() :mixed{
$file = '/etc/passwd'.chr(0).'asdf';

$opt_choices = vec[
  dict['ssl' => dict['cafile' => $file]],
  dict['ssl' => dict['capath' => $file]],
  dict['ssl' => dict['local_cert' => $file]],
];

$errno = null;
$errstr = null;
$socket = stream_socket_server('tcp://localhost:0', inout $errno, inout $errstr);
$name = stream_socket_get_name($socket, false);

$port = explode(":", $name)[1];

foreach ($opt_choices as $opts) {
  $ctx = stream_context_create($opts);
  $sock = stream_socket_client(
    sprintf('tls://localhost:%d', $port),
    inout $errno,
    inout $errstr,
    600.0,
    STREAM_CLIENT_CONNECT,
    $ctx
  );
  var_dump($sock);
}

fclose($socket);
}
