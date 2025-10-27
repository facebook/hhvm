<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function generate($arr) :mixed{
  for ($i = 0; $i < 2000; $i++) {
    $arr = vec[$arr, $arr];
  }
  return $arr;
}

class C {
  public vec $m_arr;
}

<<__EntryPoint>>
function main() :mixed{
  $arr = generate(vec[null]);
  $obj = new C;
  $obj->m_arr = $arr;

  echo "=== Unlimited depth ===\n";
  $result = objprof_get_data();
  $bytes1 = $result['C']['bytes'] ?? 0;
  echo "C bytes: " . ($result['C']['bytes'] ?? 0) . "\n";

  echo "\n=== Max depth 100 ===\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 100);
  $bytes2 = $result['C']['bytes'] ?? 0;
  echo "C bytes: " . ($result['C']['bytes'] ?? 0) . "\n";

  echo "\n=== Max depth 10 ===\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 10);
  $bytes3 = $result['C']['bytes'] ?? 0;
  echo "C bytes: " . ($result['C']['bytes'] ?? 0) . "\n";

  echo "\n=== Max visits 100 ===\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0, 100);
  $bytes4 = $result['C']['bytes'] ?? 0;
  echo "C bytes: " . ($result['C']['bytes'] ?? 0) . "\n";

  echo "\n=== Max visits 10 ===\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0, 10);
  $bytes5 = $result['C']['bytes'] ?? 0;
  echo "C bytes: " . ($result['C']['bytes'] ?? 0) . "\n";

  echo "\n=== Verification ===\n";
  // Verify depth limits: more depth = more bytes
  echo "bytes(depth=10) < bytes(depth=100): " . ($bytes3 < $bytes2 ? "true" : "false") . "\n";
  echo "bytes(depth=100) < bytes(unlimited): " . ($bytes2 < $bytes1 ? "true" : "false") . "\n";

  // Verify visit limits: more visits = more bytes
  echo "bytes(visits=10) < bytes(visits=100): " . ($bytes5 < $bytes4 ? "true" : "false") . "\n";
  echo "bytes(visits=100) < bytes(unlimited): " . ($bytes4 < $bytes1 ? "true" : "false") . "\n";

  __hhvm_intrinsics\launder_value($obj);
}
