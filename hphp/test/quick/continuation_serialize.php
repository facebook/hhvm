<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  // Make new wait handles with valid values.
  $srwh = StaticResultWaitHandle::create(42);
  $r = $srwh->join();
  var_dump($r); // Shows 42 correctly.

  $sewh = StaticExceptionWaitHandle::create(new Exception("Hi!"));
  try {
    $r = $sewh->join();
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
  $sewh = null; // Let it go
  $srwh = unserialize($s1);
  $srwh = null; // Let it go
}

main();
