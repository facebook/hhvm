<?hh

function foo($x) :mixed{
 var_dump($x);
 }
function test() :mixed{
  $data = darray['bar' => darray[]];
  $data['bar']['baz'] = 1;
  foo($data);
}

<<__EntryPoint>>
function main_239() :mixed{
test();
}
