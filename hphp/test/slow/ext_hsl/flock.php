<?hh
use namespace HH\Lib\_Private\_OS;

function try_lock(string $path, int $type): void {
  $fd = _OS\open($path, _OS\O_RDWR, 0644);
  try {
    _OS\flock($fd, $type | _OS\LOCK_NB);
    print("- OK\n");
  } catch (_OS\ErrnoException $e) {
    printf(
      "- FAIL: %s\n",
      $e->getCode() === _OS\EAGAIN ? 'EAGAIN' : (string) $e->getCode(),
    );
  }
  _OS\close($fd);
}

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/'.'hsl-os-flock';
  try {
    $fd = _OS\open($path, _OS\O_CREAT | _OS\O_RDWR | _OS\O_EXCL, 0644);
    print("Acquiring first exclusive lock\n");
    _OS\flock($fd, _OS\LOCK_EX);
    print("Acquiring second lock\n");
    try_lock($path, _OS\LOCK_EX);
    print("Re-trying acquiring second lock\n");
    try_lock($path, _OS\LOCK_EX);
    print("Trying to get a shared lock\n");
    try_lock($path, _OS\LOCK_SH);
    print("Downgrading exclusive lock to shared lock\n");
    _OS\flock($fd, _OS\LOCK_SH);
    print("Trying to get a second shared lock\n");
    try_lock($path, _OS\LOCK_SH);
    print("Trying to get an exclusive lock while shared lock open\n");
    try_lock($path, _OS\LOCK_EX);
    print("Unlocking\n");
    _OS\flock($fd, _OS\LOCK_UN);
    print("Opening a second fd with exclusive lock\n");
    try_lock($path, _OS\LOCK_EX);
    print("Re-acquiring exclusive lock\n");
    _OS\flock($fd, _OS\LOCK_EX);
    print("Attempting second lock\n");
    try_lock($path, _OS\LOCK_SH);
    print("Closing first fd\n");
    _OS\close($fd);
    print("Reopening with exclusive lock\n");
    try_lock($path, _OS\LOCK_EX);
  } finally {
    unlink($path);
  }
}
