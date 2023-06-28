<?hh

class T {
 function __toString()[] :mixed{
 return 123;
}
}

 <<__EntryPoint>>
function main_1278() :mixed{
$obj = new T();
 var_dump($obj);
}
