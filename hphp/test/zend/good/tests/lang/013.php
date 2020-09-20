<?hh <<__EntryPoint>> function main(): void {
error_reporting(0);
$a="echo \"Hello\";";
eval('function eval_func() { '.$a.' }');
eval_func();
}
