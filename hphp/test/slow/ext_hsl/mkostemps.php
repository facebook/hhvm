<?hh
use namespace HH\Lib\_Private\_OS;

// This file:
// - tests that `mkostemps` works as expected
// - tests that `mkostemps` can be used to implement related functions,
//   such as `mkstemp`
// - only uses six X's in patterns; other numbers of X's are not portable.

<<__EntryPoint>>
function main(): void {
  $prefix = sys_get_temp_dir().'/hsl-mkostemps-test';
  $pattern = $prefix.'XXXXXX';

  // Emulate mkstemp
  list($fd, $path) = _OS\mkostemps($pattern, 0, 0);
  _OS\write($fd, "foo");
  _OS\lseek($fd, -1, _OS\SEEK_CUR);
  _OS\write($fd, "bar");
  var_dump(vec[
    'mkstemp',
    $fd,
    $path,
    substr($path, 0, strlen($prefix)) === $prefix,
    strlen($path) === strlen($pattern),
    $path !== $pattern,
    file_get_contents($path), // string(5) "fobar" due to lseek
  ]);
  unlink($path);

  // Emulate mkstemps
  list($fd, $path) = _OS\mkostemps($pattern.'.txt', 4, 0);
  var_dump(vec[
    'mkstemps',
    $fd,
    $path,
    substr($path, 0, strlen($prefix)) === $prefix,
    strlen($path) === strlen($pattern.'.txt'),
    $path !== $pattern.'.txt',
  ]);
  unlink($path);

  // Emulate mkostemp
  list($fd, $path ) = _OS\mkostemps($pattern, 0, _OS\O_APPEND);
  _OS\write($fd, "foo");
  _OS\lseek($fd, -1, _OS\SEEK_CUR);
  _OS\write($fd, "bar");
  var_dump(vec['mkostemp', file_get_contents($path)]); // foobar, O_APPEND re-seeks to EOF
  unlink($path);

  // Test *all* the features
  list($fd, $path ) = _OS\mkostemps($pattern.'.txt', 4, _OS\O_APPEND);
  _OS\write($fd, "foo");
  _OS\lseek($fd, -1, _OS\SEEK_CUR);
  _OS\write($fd, "bar");
  var_dump(vec[
    'mkostemps',
    $fd,
    $path,
    substr($path, 0, strlen($prefix)) === $prefix,
    strlen($path) === strlen($pattern.'.txt'),
    $path !== $pattern,
    file_get_contents($path),
  ]);
  unlink($path);
}
