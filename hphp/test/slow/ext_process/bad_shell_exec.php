<?php


<<__EntryPoint>>
function main_bad_shell_exec() {
var_dump(``);
var_dump(`/invalid_path/`);
var_dump(shell_exec('cat '.__DIR__.'/../../sample_dir/empty'));
}
