<?hh

function main() :mixed{
  $a = new stdClass();
  $ro = new ReflectionObject($a);
  var_dump($ro->getMethods());

  $rc = new ReflectionClass(get_class($a));

  try {
    $ro = new ReflectionObject(get_class($a));
  } catch (Exception $e) {
    // Throwing an exception is different from PHP5, which allows for the
    // ReflectionObject to be created but then fatal hard.
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}


<<__EntryPoint>>
function main_reflection_object() :mixed{
main();
}
