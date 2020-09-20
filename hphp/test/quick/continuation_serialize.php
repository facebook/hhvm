<?hh


async function static_result_create($result) {
  return $result;
}

async function static_exception_create($exception) {
  throw $exception;
}

<<__EntryPoint>> function main(): void {
  // Make new wait handles with valid values.
  $srwh = static_result_create(42);
  $r = HH\Asio\join($srwh);
  var_dump($r); // Shows 42 correctly.

  $sewh = static_exception_create(new Exception("Hi!"));
  try {
    $r = HH\Asio\join($sewh);
  } catch (Exception $e) {
    var_dump($e->getMessage()); // Shows "Hi!" correctly.
  }

  // Serialize the handles and let them go.
  $s1 = serialize($srwh);
  $s2 = serialize($sewh);
  var_dump($s1);
  var_dump($s2);
  $srwh = null;
  $erwh = null;

  // Deserialize the handles in the reverse order, so they lay over
  // each other's memory. We want to confirm that all fields are
  // initialized correctly and that the destructor does not segfault.
  $sewh = unserialize($s2);
  var_dump($sewh);
  $sewh = null; // Let it go
  $srwh = unserialize($s1);
  var_dump($srwh);
  $srwh = null; // Let it go

  // Confirm that we can't deserialize one of these as well.
  $c1 = unserialize("O:9:\"Generator\":0:{}");
  var_dump($c1);
}
