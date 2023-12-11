<?hh

function a() :mixed{
  $input = vec["php", 4.0, vec["green", "red"]];
  $result = array_reverse($input);
  var_dump($result);
  $result_keyed = array_reverse($input, true);
  var_dump($result_keyed);
}

function b() :mixed{
  $input = dict["php" => 4.0, 10 => 5.0, "blab" =>"b"];
  $result = array_reverse($input);
  var_dump($result);
}



<<__EntryPoint>>
function main_array_reverse() :mixed{
a();
b();
}
