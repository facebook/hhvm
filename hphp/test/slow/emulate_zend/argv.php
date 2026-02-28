<?hh


<<__EntryPoint>>
function main_argv() :mixed{
  $to_exec = HH\__internal\hhvm_binary().' --php -n '.__DIR__.'/argv.inc foo bar';
var_dump($to_exec);
$return_var = -1;
system($to_exec, inout $return_var);
}
