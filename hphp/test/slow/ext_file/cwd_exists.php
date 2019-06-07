<?hh


<<__EntryPoint>>
function main_cwd_exists() {
var_dump(file_exists('/'));
var_dump(file_exists('.'));
var_dump(file_exists('./'));
}
