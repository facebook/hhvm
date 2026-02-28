<?hh
<<__EntryPoint>>
function entrypoint_autoload_006(): void {

  var_dump(interface_exists('autoload_interface', false));
  var_dump(class_exists('autoload_implements', false));

  $o = new autoload_implements;
  var_dump($o);
  var_dump($o is autoload_interface);
  unset($o);

  var_dump(interface_exists('autoload_interface', false));
  var_dump(class_exists('autoload_implements', false));

  echo "===DONE===\n";
}
