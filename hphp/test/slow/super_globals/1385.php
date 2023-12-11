<?hh

function test() :mixed{
  $_POST = dict['HELLO' => 1];
}

<<__EntryPoint>>
function main_1385() :mixed{
test();
var_dump($_POST);
}
