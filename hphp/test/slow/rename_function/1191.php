<?hh

function one() {
 echo 'one';
}

<<__EntryPoint>>
function main_1191() {
fb_rename_function('one', 'two');
fb_rename_function('two', 'three');
three();
}
