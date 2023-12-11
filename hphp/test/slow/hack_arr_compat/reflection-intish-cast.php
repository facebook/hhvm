<?hh

<<__EntryPoint>>
function main () :mixed{
  try {
    var_dump(new ReflectionClass('42'));
  } catch (Exception $e) {}

  var_dump(call_user_func(vec['42', 'foo']));
  $x = new stdClass();
  $x->{42} = 'foo';

  try {
    var_dump((new ReflectionObject($x))->getProperties());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    var_dump((new ReflectionProperty($x, '42')));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
