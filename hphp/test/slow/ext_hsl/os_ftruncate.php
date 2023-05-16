<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $temp = sys_get_temp_dir().'/'.'hsl_os_ftruncate';
  touch($temp);
  $fd = _OS\open($temp, _OS\O_WRONLY);

  _OS\write($fd, 'foobarbaz');
  var_dump(file_get_contents($temp)); // "foobarbaz"

  _OS\ftruncate($fd, 6);
  var_dump(file_get_contents($temp)); // "foobar"

  _OS\ftruncate($fd, 3);
  var_dump(file_get_contents($temp)); // "foo"

  _OS\ftruncate($fd, 0);
  var_dump(file_get_contents($temp)); // ""

  _OS\ftruncate($fd, 0);
  _OS\write($fd, 'herpderp');
  var_dump(bin2hex(file_get_contents($temp))); // "<nulls>herpderp"

  _OS\ftruncate($fd, 0);
  var_dump(_OS\lseek($fd, 0, _OS\SEEK_SET));
  _OS\write($fd, 'herpderp');
  var_dump(file_get_contents($temp)); // "herpderp"

  _OS\ftruncate($fd, 0);
  var_dump(_OS\lseek($fd, 0, _OS\SEEK_END));
  _OS\write($fd, 'herpderp');
  var_dump(file_get_contents($temp)); // "herpderp"

  _OS\ftruncate($fd, 4);
  var_dump(_OS\lseek($fd, 0, _OS\SEEK_SET));
  _OS\write($fd, 'herpderp');
  var_dump(file_get_contents($temp)); // "herpderp"

  _OS\ftruncate($fd, 4);
  var_dump(_OS\lseek($fd, 0, _OS\SEEK_END));
  _OS\write($fd, 'herpderp');
  var_dump(file_get_contents($temp)); // "herpherpderp"


  try {
    _OS\ftruncate($fd, -1);
  } catch (_OS\ErrnoException $e) {
    if ($e->getCode() === _OS\EINVAL) {
      var_dump("EINVAL on negative length");
    } else {
      throw $e;
    }
  }

  unlink($temp);
}
