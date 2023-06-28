<?hh


<<__EntryPoint>>
function main_bad_shell_exec() :mixed{
var_dump(shell_exec(''));
var_dump(shell_exec('/invalid_path/'));
var_dump(shell_exec('cat '.__DIR__.'/../../sample_dir/empty'));
}
