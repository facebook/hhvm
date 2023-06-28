<?hh


<<__EntryPoint>>
function main_bad_func() :mixed{
var_dump(ob_start('bad_func'));
var_dump('this prints');
var_dump(ob_end_clean());
}
