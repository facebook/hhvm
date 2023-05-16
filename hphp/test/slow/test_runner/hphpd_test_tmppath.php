<?hh

<<__EntryPoint>>
function main(): void {
  $root = sys_get_temp_dir();
  $path = sys_get_temp_dir().'/'.'foo.txt';
  var_dump('root does not end with /', strrpos($root, '/') !== strlen($root) - 1);
  var_dump('path starts with root', strpos($path, $root) === 0);
  var_dump(
    'requested path is leaf',
    strrpos($path, '/foo.txt') === strlen($path) - strlen('/foo.txt'),
  );
  var_dump('no double slashes in root', strpos($root, '//') === false);
  var_dump('no double slashes in path', strpos($path, '//') === false);
}
