<?hh
const FOO=1;


<<__EntryPoint>>
function main_literal_string_index() :mixed{
var_dump('abc'[1]);
var_dump('abc'[FOO]);
var_dump('abcdef'[1 + 1]);
$foo = 1;
var_dump('abc'[$foo]);
}
