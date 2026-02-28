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

  // Test objprof_get_data_with_graph_stats
  echo "\n=== objprof_get_data_with_graph_stats ===\n";

  echo "\n--- Unlimited depth and visits ---\n";
  $result = objprof_get_data_with_graph_stats();
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  $unlimited_nodes = $result['nodes_visited'];
  $unlimited_depth = $result['max_depth_seen'];

  echo "\n--- Max depth 10 ---\n";
  $result = objprof_get_data_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 10);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "max_depth_seen <= 10: " . ($result['max_depth_seen'] <= 10 ? "true" : "false") . "\n";

  echo "\n--- Max visits 100 ---\n";
  $result = objprof_get_data_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 0, 100);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "nodes_visited <= 100: " . ($result['nodes_visited'] <= 100 ? "true" : "false") . "\n";
  echo "nodes_visited < unlimited: " . ($result['nodes_visited'] < $unlimited_nodes ? "true" : "false") . "\n";

  // Test objprof_get_data_extended_with_graph_stats
  echo "\n=== objprof_get_data_extended_with_graph_stats ===\n";

  echo "\n--- Unlimited depth and visits ---\n";
  $result = objprof_get_data_extended_with_graph_stats();
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";

  echo "\n--- Max depth 50 ---\n";
  $result = objprof_get_data_extended_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 50);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "max_depth_seen <= 50: " . ($result['max_depth_seen'] <= 50 ? "true" : "false") . "\n";

  echo "\n--- Max visits 50 ---\n";
  $result = objprof_get_data_extended_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 0, 50);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "nodes_visited <= 50: " . ($result['nodes_visited'] <= 50 ? "true" : "false") . "\n";

  // Test objprof_get_paths_with_graph_stats
  echo "\n=== objprof_get_paths_with_graph_stats ===\n";

  echo "\n--- Unlimited depth and visits ---\n";
  $result = objprof_get_paths_with_graph_stats();
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";

  echo "\n--- Max depth 20 ---\n";
  $result = objprof_get_paths_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 20);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "max_depth_seen <= 20: " . ($result['max_depth_seen'] <= 20 ? "true" : "false") . "\n";

  echo "\n--- Max visits 25 ---\n";
  $result = objprof_get_paths_with_graph_stats(OBJPROF_FLAGS_DEFAULT, vec[], 0, 25);
  echo "nodes_visited: " . $result['nodes_visited'] . "\n";
  echo "max_depth_seen: " . $result['max_depth_seen'] . "\n";
  echo "nodes_visited <= 25: " . ($result['nodes_visited'] <= 25 ? "true" : "false") . "\n";

  __hhvm_intrinsics\launder_value($obj);
}
