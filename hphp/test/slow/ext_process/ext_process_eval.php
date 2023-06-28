<?hh


<<__EntryPoint>>
function main_ext_process_eval() :mixed{
$pid = pcntl_fork();
if ($pid == 0) {
  eval('function a() { return 42; }');
  $a = a();
  print $a . "\n";
  exit(5);
}
$status = null;
pcntl_wait(inout $status);
eval('function a() { return 53; }');
$a = a();
print $a . "\n";
}
