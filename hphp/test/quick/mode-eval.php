<?hh

<<__EntryPoint>>
function main() {
  $rv = 0;
  system(PHP_BINARY." -m eval 'echo \"Hello, World!\\n\";'", inout $rv);
  var_dump($rv);
  system(PHP_BINARY." --php -n -r 'echo \"Hello, World!\\n\";'", inout $rv);
  var_dump($rv);
  $out = shell_exec(PHP_BINARY." --php -n -i");
  var_dump(strpos($out, 'HHVM Version =>') !== false);
}
