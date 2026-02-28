<?hh

function f($name, $unique_id=false, $id=null) :mixed{
  $id = $id ? $id : ($unique_id ? uniqid($name) : $name);
  return $id;
  }
function test($a, $b, $c) :mixed{
  return f($name = 'status', $unique_id = true, $id = 'status_active');
  }

<<__EntryPoint>>
function main_1832() :mixed{
var_dump(test(1,2,3));
}
