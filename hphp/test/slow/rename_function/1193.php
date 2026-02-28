<?hh

function one() :mixed{
 echo 'one';
}

<<__EntryPoint>>
function main_1193() :mixed{
var_dump(fb_rename_function('one', 'two'));
}
