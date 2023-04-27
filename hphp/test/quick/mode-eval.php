<?hh

<<__EntryPoint>>
function main() {
  $rv = 0;
  system(
    HH\__internal\hhvm_binary()." -m eval 'echo \"Hello, World!\\n\";'",
    inout $rv
  );
  var_dump($rv);
  system(
    HH\__internal\hhvm_binary()." --php -n -r 'echo \"Hello, World!\\n\";'",
    inout $rv
  );
  var_dump($rv);
  $out = shell_exec(HH\__internal\hhvm_binary()." --php -n -i");
  var_dump(strpos($out, 'HHVM Version =>') !== false);
}
