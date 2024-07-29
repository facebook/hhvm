<?hh

function test() :mixed{
  \HH\global_set('_POST', dict['HELLO' => 1]);
}

<<__EntryPoint>>
function main_1385() :mixed{
test();
var_dump(\HH\global_get('_POST'));
}
