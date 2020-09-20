<?hh

function one() {
 echo 'one';
}

<<__EntryPoint>>
function main_1193() {
var_dump(fb_rename_function('one', 'two'));
}
