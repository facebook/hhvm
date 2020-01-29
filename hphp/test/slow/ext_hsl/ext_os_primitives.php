<?hh // strict

use namespace HH\Lib\OS;
use namespace HH\Lib\_Private\OS as _OS;
<<__EntryPoint>>
function main(): void {
  $filename = sys_get_temp_dir()."/".\bin2hex(\random_bytes(8));
  $fd = _OS\open($filename, OS\O_CREAT | OS\O_WRONLY, 0644);
  var_dump($fd);
  _OS\write($fd, "Hello, world\n");
  _OS\close($fd);
  var_dump(\file_get_contents($filename));
  var_dump($fd);
  printf("Expecting exception EBADF\n");
  try {
    _OS\write($fd, "Foo bar\n");
  } catch (\Throwable $e) {
    printf(
      "> Caught exception %s with code (%d): %s\n",
      get_class($e),
      $e->getCode(),
      $e->getMessage(),
    );
  }
  printf("Expecting exception EEXIST\n");
  try {
    $fd = _OS\open($filename, OS\O_CREAT | OS\O_EXCL | OS\O_WRONLY, 0644);
  } catch (\Throwable $e) {
    printf(
      "> Caught exception %s with code (%d): %s\n",
      get_class($e),
      $e->getCode(),
      $e->getMessage(),
    );
  }

  unlink($filename);
}
