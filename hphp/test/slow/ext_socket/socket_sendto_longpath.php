<?hh

async function doTest(string $path): Awaitable<void> {
  $port = 0; // junk, but mandatory
  $server = socket_create(AF_UNIX, SOCK_DGRAM, 0);
  print("- Bind result: ");
  $bound = socket_bind($server, $path);
  \var_dump($bound);
  if (!$bound) {
    return;
  }

  printf("- Server path %s\n", file_exists($path) ? 'exists' : 'does not exist');
  $peer_path = '';
  socket_getsockname($server, inout $peer_path, inout $port);
  printf("- Bound path %s requested path\n", $peer_path === $path ? 'equals' : 'does not equal');

  $client = socket_create(AF_UNIX, SOCK_DGRAM, 0);
  print("- socket_sendto result: ");
  var_dump(socket_sendto($client, 'hello', strlen('hello'), MSG_EOR, $path));
  $buf = '';
  $recv_path = '__JUNK__'; // unset for unix sockets
  print("- socket_recvfrom result: ");
  var_dump(socket_recvfrom($server, inout $buf, 5, MSG_DONTWAIT, inout $recv_path, inout $port));
  printf("- read data: %s\n", $buf);


  if (file_exists($path)) {
    unlink($path);
    return;
  }

  // Silently truncated?
  socket_getsockname($server, inout $path, inout $port);
  if (file_exists($path)) {
    printf("- Deleting truncated path %s\n", $path);
    unlink($path);
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  // Size of sockaddr_un.sun_path field
  // Linux: man 7 unix
  // MacOS: man 4 unix
  switch (php_uname('s')) {
    case 'Darwin':
      $max_len = 104;
      break;
    case 'Linux':
      $max_len = 108;
      break;
    default:
      invariant_violation("This test needs updating to support the current platform");
  }

  $junk = \bin2hex(\random_bytes($max_len)); // much longer than the safe value
  $path = __SystemLib\hphp_test_tmppath("sendto-test-$junk");
  invariant(strlen($path) > $max_len, "Path is too short :/");

  print("Definitely too-long path\n");
  await doTest($path);
  print("Safe path\n");
  await doTest(substr($path, 0, $max_len - 1));
  print("Exactly max len\n");
  await doTest(substr($path, 0, $max_len));
  print("Just too big\n");
  await doTest(substr($path, 0, $max_len + 1));
}
