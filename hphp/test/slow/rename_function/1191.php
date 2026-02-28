<?hh

function one() :mixed{
 echo 'one';
}

<<__EntryPoint>>
function main_1191() :mixed{
fb_rename_function('one', 'two');
fb_rename_function('two', 'three');
three();
}
