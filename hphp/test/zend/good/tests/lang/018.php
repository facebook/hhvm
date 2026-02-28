<?hh
<<__EntryPoint>> function main(): void {
error_reporting(0);

$message = "echo \"hey\n\";";

for ($i=0; $i<10; $i++) {
  $func = 'hey'.$i;
  eval('<<__DynamicallyCallable>> function '.$func.'() { '.$message.' }');
  HH\dynamic_fun($func)();
  echo $i."\n";
}
}
