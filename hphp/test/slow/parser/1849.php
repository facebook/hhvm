<?hh

function foo() :mixed{
 return varray[1, 2, 3];
}

 <<__EntryPoint>>
function main_1849() :mixed{
var_dump(foo()[2]);
}
