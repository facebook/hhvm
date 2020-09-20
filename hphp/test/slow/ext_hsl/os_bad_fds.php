<?hh

use namespace HH\Lib\_Private\_OS;

class NotAnFD {}

function t(string $label, (function():void) $fun): void {
  $expected = dict[
    _OS\EBADF => 'EBADF',
    _OS\ENOTSOCK => 'ENOTSOCK',
    _OS\ESPIPE => 'ESPIPE',
  ];
  printf("%s:\n", $label);
  try {
    $fun();
    print("- OK\n");
  } catch (Throwable $e) {
    printf(
      "- Caught exception: %s\n",
      $expected[$e->getCode()] ?? sprintf('%d (%s - "%s")', $e->getCode(), \get_class($e), $e->getMessage()),
    );
  }
}

<<__EntryPoint>>
function main(): void {
  // Testing a few functions, not all: assumption is if these
  // pass, they're protected by shared code paths, e.g.
  // parameter type verification or HSLFileDescriptor's accessors

  $file_fd = _OS\open('/dev/null', _OS\O_RDONLY);
  $closed_fd = _OS\open('/dev/null', _OS\O_RDONLY);
  _OS\close($closed_fd);
  list($r_pipe, $w_pipe) = _OS\pipe();

  print("--- closed FD ---\n");
  t('close', () ==> _OS\close($closed_fd));
  t('lseek', () ==> _OS\lseek($closed_fd, 0, _OS\SEEK_SET));
  t('write', () ==> _OS\write($closed_fd, 'foo'));
  t('getpeername', () ==> _OS\getpeername($closed_fd));

  print("--- wrong kind of FD ---\n");
  t('getpeername', () ==> _OS\getpeername($file_fd));
  t('lseek', () ==> _OS\lseek($w_pipe, 123, _OS\SEEK_CUR));

  print("--- Not an FD ---\n");
  // if engine exceptions are disabled, the first one of these should fatal
  printf("PHP7 engine exceptions (required): %s\n", ini_get('hhvm.php7.engine_exceptions'));
  t('close', () ==> _OS\close(new NotAnFD()));
  t('lseek', () ==> _OS\lseek(new NotAnFD(), 0, _OS\SEEK_SET));
  t('getpeername', () ==> _OS\getpeername(new NotAnFD()));

  print("--- null ---\n");
  t('close', () ==> _OS\close(null));
  t('lseek', () ==> _OS\lseek(null, 0, _OS\SEEK_SET));
  t('getpeername', () ==> _OS\getpeername(null));
}
