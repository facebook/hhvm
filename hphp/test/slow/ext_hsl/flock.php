<?hh
use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/hsl-os-flock-'.bin2hex(random_bytes(16));
  try {
    $fda = _OS\open($path, _OS\O_CREAT | _OS\O_RDWR | _OS\O_EXCL, 0644);
    print("Acquiring first lock\n");
    _OS\flock($fda, _OS\LOCK_EX);
    $fdb = _OS\open($path, _OS\O_RDWR, 0644);
    print("Acquiring second lock\n");
    try {
      _OS\flock($fdb, _OS\LOCK_EX | _OS\LOCK_NB);
    } catch (_OS\ErrnoException $e) {
      printf(
        "Caught errno %d - EAGAIN? %s\n",
        $e->getCode(),
        $e->getCode() === _OS\EAGAIN ? 'yes' : 'no'
      );
    }
  } finally {
    unlink($path);
  }
}
