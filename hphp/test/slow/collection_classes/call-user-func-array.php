<?hh
class C {
  public static function foo(...$args) {
    var_dump(__FUNCTION__, $args);
  }
}
function main() {
  $cufa = 'call_user_func_array';
  if (\HH\global_get('argc') > 1000000000) {
    $cufa = 'call_user_func';
  }
  call_user_func_array('var_dump', darray[4 => 123, 6 => 456]);
  echo "\n";
  call_user_func_array('C::foo', darray[4 => 123, 6 => 456]);
  echo "\n";
  $cufa('var_dump', darray[4 => 123, 6 => 456]);
  echo "\n";
  $cufa('C::foo', darray[4 => 123, 6 => 456]);
  echo "\n";
  call_user_func_array('var_dump', Vector {123, 456});
  echo "\n";
  call_user_func_array('C::foo', Vector {123, 456});
  echo "\n";
  $cufa('var_dump', Vector {123, 456});
  echo "\n";
  $cufa('C::foo', Vector {123, 456});
  echo "\n";
  call_user_func_array('var_dump', Map {2 => 123});
  echo "\n";
  call_user_func_array('C::foo', Map {2 => 123});
  echo "\n";
  $cufa('var_dump', Map {2 => 123});
  echo "\n";
  $cufa('C::foo', Map {2 => 123});
  echo "\n";
  call_user_func_array('var_dump', Map {4 => 123, 6 => 456});
  echo "\n";
  call_user_func_array('C::foo', Map {4 => 123, 6 => 456});
  echo "\n";
  $cufa('var_dump', Map {4 => 123, 6 => 456});
  echo "\n";
  $cufa('C::foo', Map {4 => 123, 6 => 456});
  echo "\n";
  call_user_func_array('var_dump', Set {123});
  echo "\n";
  call_user_func_array('C::foo', Set {123});
  echo "\n";
  $cufa('var_dump', Set {123});
  echo "\n";
  $cufa('C::foo', Set {123});
  echo "\n";
  call_user_func_array('var_dump', Pair {11, 'a'});
  echo "\n";
  call_user_func_array('C::foo', Pair {11, 'a'});
  echo "\n";
  $cufa('var_dump', Pair {11, 'a'});
  echo "\n";
  $cufa('C::foo', Pair {11, 'a'});
  echo "\n";
}


<<__EntryPoint>>
function main_call_user_func_array() {
main();
}
