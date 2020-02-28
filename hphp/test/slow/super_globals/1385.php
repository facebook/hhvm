<?hh

function test() {
  $_POST = darray['HELLO' => 1];
}

<<__EntryPoint>>
function main_1385() {
test();
var_dump($_POST);
}
