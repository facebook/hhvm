<?hh

use namespace HH\Lib\_Private\_OS;

function tell(HH\Lib\OS\FileDescriptor $fd): int {
  return _OS\lseek($fd, 0, _OS\SEEK_CUR);
}

<<__EntryPoint>>
function main(): void {
  $temp = tempnam(\sys_get_temp_dir(), 'hsl_lseek_test');
  file_put_contents($temp, 'foobarbaz');
  $fd = _OS\open($temp, _OS\O_RDONLY);
  \var_dump(tell($fd)); // 0
  \var_dump(_OS\lseek($fd, 3, SEEK_CUR)); // 3
  \var_dump(vec[tell($fd), _OS\read($fd, 3)]); // 3, bar
  \var_dump(_OS\lseek($fd, 2, SEEK_SET)); // 2
  \var_dump(vec[tell($fd), _OS\read($fd, 3)]); // 2, oba
  \var_dump(_OS\lseek($fd, -2, SEEK_CUR)); // 3
  \var_dump(vec[tell($fd), _OS\read($fd, 3)]); // 3, bar
  \var_dump(_OS\lseek($fd, -2, SEEK_CUR)); // 4
  \var_dump(_OS\read($fd, 9)); // arbaz
  \var_dump(_OS\lseek($fd, -3, SEEK_END)); // 6
  \var_dump(_OS\read($fd, 3)); // baz

  _OS\close($fd);
  \unlink($temp);
}
