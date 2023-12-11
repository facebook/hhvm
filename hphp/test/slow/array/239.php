<?hh

function foo($x) :mixed{
 var_dump($x);
 }
function test() :mixed{
  $data = dict['bar' => dict[]];
  $data['bar']['baz'] = 1;
  foo($data);
}

<<__EntryPoint>>
function main_239() :mixed{
test();
}
