<?hh
<<__EntryPoint>> function main(): void {
error_reporting(0);

$message = "echo \"hey\n\";";

for ($i=0; $i<10; $i++) {
  $func = 'hey'.$i;
  eval('function '.$func.'() { '.$message.' }');
  $func();
  echo $i."\n";
}
}
