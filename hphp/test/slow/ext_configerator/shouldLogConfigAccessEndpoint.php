<?hh

<<__EntryPoint>>
function main(): void {
  // Clear the bloom filter before each test
  ConfigeratorExtensionApi::resetConfigAccessExposures();

  // Test case 1: First call with a combination should return true
  $endpoint1 = "api/v1/users";
  $configName1 = "user_config";
  $result1 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint1, $configName1);
  echo "Test 1: First call with endpoint1 + configName1: " . ($result1 ? "true" : "false") . "\n";

  // Test case 2: Second call with the same combination should return false
  $result2 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint1, $configName1);
  echo "Test 2: Second call with endpoint1 + configName1: " . ($result2 ? "true" : "false") . "\n";

  // Test case 3: Different endpoint, same configName should return true
  $endpoint2 = "api/v1/posts";
  $result3 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint2, $configName1);
  echo "Test 3: First call with endpoint2 + configName1: " . ($result3 ? "true" : "false") . "\n";

  // Test case 4: Same endpoint, different configName should return true
  $configName2 = "post_config";
  $result4 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint1, $configName2);
  echo "Test 4: First call with endpoint1 + configName2: " . ($result4 ? "true" : "false") . "\n";

  // Test case 5: Second call with different endpoint, same configName should return false
  $result5 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint2, $configName1);
  echo "Test 5: Second call with endpoint2 + configName1: " . ($result5 ? "true" : "false") . "\n";

  // Test case 6: Second call with same endpoint, different configName should return false
  $result6 = ConfigeratorExtensionApi::shouldLogConfigAccessEndpoint($endpoint1, $configName2);
  echo "Test 6: Second call with endpoint1 + configName2: " . ($result6 ? "true" : "false") . "\n";
}
