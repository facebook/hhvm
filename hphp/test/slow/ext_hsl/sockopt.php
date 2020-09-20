<?hh

use namespace HH\Lib\_Private\_OS;

function dump(\HH\Lib\OS\FileDescriptor $fd, string $name): void {
  try {
    $type = _OS\getsockopt_int($fd, _OS\SOL_SOCKET, _OS\SO_TYPE);
    printf("%s SO_TYPE: %d\n", $name, $type);
  } catch (_OS\ErrnoException $e) {
    printf("%s SO_TYPE: %s\n", $name, $e->getMessage());
  }
}

function getandset(): void {
  print("***** GETTING AND SETTING *****\n");
  $file = _OS\open('/dev/null', _OS\O_RDONLY);
  $tcp = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  $udp = _OS\socket(_OS\PF_INET, _OS\SOCK_DGRAM, 0);
  $unix = _OS\socket(_OS\PF_UNIX, _OS\SOCK_STREAM, 0);
  dump($file, 'file');
  dump($tcp, 'tcp');
  dump($udp, 'udp');
  dump($unix, 'unix');

  printf(
    "SO_REUSEADDR: %d\n",
    _OS\getsockopt_int($tcp, _OS\SOL_SOCKET, _OS\SO_REUSEADDR),
  );
  _OS\setsockopt_int($tcp, _OS\SOL_SOCKET, _OS\SO_REUSEADDR, 1);
  printf(
    "SO_REUSEADDR: %d\n",
    _OS\getsockopt_int($tcp, _OS\SOL_SOCKET, _OS\SO_REUSEADDR),
  );
}

function reuseaddr(): void {
  // Nothing special about SO_REUSEADDR, except it's easy to write a test to
  // make sure it actually has an effect
  print("***** TESTING SO_REUSEADDR WORKS *****\n");
  $server = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  _OS\bind($server, new _OS\sockaddr_in(0, _OS\INADDR_LOOPBACK));
  // as we requested port 0, we actually got a random port. Find out what it was
  $address = _OS\getsockname($server) as _OS\sockaddr_in;
  _OS\listen($server, 16);
  $client = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  _OS\connect($client, $address);
  _OS\accept($server);
  _OS\write($client, "Hello\n");
  _OS\close($server);

  $server = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  try {
    _OS\bind($server, $address);
  } catch (_OS\ErrnoException $e) {
    printf(
      "Binding duplicate address without SO_REUSEADDR failed (expected): %s\n",
      $e->getMessage(),
    );
  }

  $server = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  _OS\setsockopt_int($server, _OS\SOL_SOCKET, _OS\SO_REUSEADDR, 1);
  _OS\bind($server, new _OS\sockaddr_in(0, _OS\INADDR_LOOPBACK));
  $address = _OS\getsockname($server) as _OS\sockaddr_in;
  _OS\listen($server, 16);
  $client = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  _OS\connect($client, $address);
  _OS\accept($server);
  _OS\write($client, "Hello\n");
  _OS\close($server);

  $server = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  _OS\setsockopt_int($server, _OS\SOL_SOCKET, _OS\SO_REUSEADDR, 1);
  _OS\bind($server, $address);
  print("Binding duplicate address with SO_REUSEADDR succeeded!\n");
}


function badsize(): void {
  printf("***** SETTING A NON-INT OPTION TO AN INT ******\n");
  $sock = _OS\socket(_OS\PF_INET, _OS\SOCK_STREAM, 0);
  // Testing assertions in the native implementation for return size
  _OS\getsockopt_int($sock, _OS\SOL_SOCKET, _OS\SO_LINGER);
  print("getsockopt_int() with SO_LINGER succeeded with a truncated result.\n");
  try {
    _OS\setsockopt_int($sock, _OS\SOL_SOCKET, _OS\SO_LINGER, 0);
    print("BAD: setsockopt_int() succeeded with SO_LINGER\n");
  } catch (_OS\ErrnoException $e) {
    printf(
      "setsockopt_int() with SO_LINGER failed (expected): %s\n",
      $e->getMessage(),
    );
  }
}

<<__EntryPoint>>
function main(): void {
  getandset();
  reuseaddr();
  badsize();
}
