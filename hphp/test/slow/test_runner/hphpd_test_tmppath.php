<?hh

<<__EntryPoint>>
function main(): void {
  $root = __SystemLib\hphp_test_tmproot();
  $path = __SystemLib\hphp_test_tmppath('foo.txt');

  var_dump('root ends with /', strrpos($root, '/') === strlen($root) - 1);
  var_dump('path starts with root', strpos($path, $root) === 0);
  var_dump(
    'requested path is leaf',
    strrpos($path, '/foo.txt') === strlen($path) - strlen('/foo.txt'),
  );
  var_dump('no double slashes in root', strpos($root, '//') === false);
  var_dump('no double slashes in path', strpos($path, '//') === false);
}
