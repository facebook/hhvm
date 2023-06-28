<?hh


<<__EntryPoint>>
function main_cwd_exists() :mixed{
var_dump(file_exists('/'));
var_dump(file_exists('.'));
var_dump(file_exists('./'));
}
