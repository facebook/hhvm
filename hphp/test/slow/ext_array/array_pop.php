<?hh

function a() :mixed{
  $input = vec["orange", "banana", "apple", "raspberryu"];
  $fruit = array_pop(inout $input);
  var_dump($input);
}

function b() :mixed{
  $input = vec["orange"];
  $fruit = array_pop(inout $input);
  array_push(inout $input, "banana");
  var_dump($input);
  var_dump($fruit);
}



<<__EntryPoint>>
function main_array_pop() :mixed{
a();
b();
}
