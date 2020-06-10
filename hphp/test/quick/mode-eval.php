<?hh

<<__EntryPoint>>
function main() {
  $rv = 0;
  system(PHP_BINARY." -m eval 'echo \"Hello, World!\\n\";'", inout $rv);
  var_dump($rv);
}
