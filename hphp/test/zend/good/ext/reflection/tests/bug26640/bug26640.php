<?hh
<<__EntryPoint>>
function entrypoint_bug26640(): void {

  $a = new ReflectionClass('autoload_class');

  if (is_object($a)) {
  	echo "OK\n";
  }
}
