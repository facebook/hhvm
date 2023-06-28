<?hh

module a.b;

<<__EntryPoint>>
function main() :mixed{
  // __SystemLib\PhpInfo functions don't belong to any module
  // but shouldn't fail deployment check
  $php_info = shell_exec(HH\__internal\hhvm_binary()." --php -n -i");
  var_dump($php_info != "");
}
