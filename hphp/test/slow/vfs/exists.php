<?hh

<<__EntryPoint>>
function main() {
  var_dump(file_exists(__DIR__.'/files/foo.dat'));
  var_dump(file_exists(__DIR__.'/files/foo.txt'));
  var_dump(file_exists(__DIR__.'/files/bar.dat'));
  var_dump(file_exists(__DIR__.'/files/bar'));
  var_dump(file_exists(__DIR__.'/files/bar/bar.dat'));

  var_dump(is_file(__DIR__.'/files/foo.dat'));
  var_dump(is_file(__DIR__.'/files/foo.txt'));
  var_dump(is_file(__DIR__.'/files/bar.dat'));
  var_dump(is_file(__DIR__.'/files/bar'));
  var_dump(is_file(__DIR__.'/files/bar/bar.dat'));

  var_dump(is_dir(__DIR__.'/files/foo.dat'));
  var_dump(is_dir(__DIR__.'/files/foo.txt'));
  var_dump(is_dir(__DIR__.'/files/bar.dat'));
  var_dump(is_dir(__DIR__.'/files/bar'));
  var_dump(is_dir(__DIR__.'/files/bar/bar.dat'));
}
