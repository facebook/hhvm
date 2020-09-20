<?hh

class T {
 function __toString() {
 return 123;
}
}

 <<__EntryPoint>>
function main_1278() {
$obj = new T();
 var_dump($obj);
}
