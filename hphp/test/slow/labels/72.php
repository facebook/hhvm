<?hh


<<__EntryPoint>>
function main_72() {
$modalité = 'extended ASCII';
 var_dump($modalité);
${
"a-b"}
 = 'dash';
 var_dump(${
"a-b"}
);
${
'a"b'}
 = 'quote';
 var_dump(${
'a"b'}
);
${
'a$b'}
 = 'dollar';
 var_dump(${
'a$b'}
);
}
