<?hh

function foo($v = test::val) {
	var_dump($v);
}
<<__EntryPoint>>
function entrypoint_class_constants_003(): void {

  $class_data = <<<DATA
<?hh
class test {
  const val = 1;
}
DATA;

  $filename = sys_get_temp_dir().'/'.'cc003.dat';
  file_put_contents($filename, $class_data);

  include $filename;

  foo();
  foo(5);

  unlink($filename);

  echo "Done\n";
}
