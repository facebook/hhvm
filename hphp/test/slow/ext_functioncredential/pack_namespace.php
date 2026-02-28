<?hh

namespace FunctionCredentialTest;

class NamespacedClass {
  public static function static_method(): \FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }

  public function instance_method(): \FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }
}

function namespaced_function(): \FunctionCredential {
  return __FUNCTION_CREDENTIAL__;
}

/**
 * Validates that a packed string matches the expected JSON format:
 * {"class_name":...,"function_name":...,"valid_until":...}:<64 hex characters>
 */
function validate_format(
  string $packed,
  ?string $expected_class,
  string $expected_func,
): bool {
  // Format: "<json>:<64_hex_chars>"
  // Find the last colon which separates JSON from the hex tag
  $lastColonPos = \strrpos($packed, ':');
  if ($lastColonPos === false || \strlen($packed) - $lastColonPos - 1 !== 64) {
    echo "  ERROR: Invalid format - expected <json>:<64 hex chars>\n";
    return false;
  }

  $jsonPart = \substr($packed, 0, $lastColonPos);
  $hexTag = \substr($packed, $lastColonPos + 1);

  // Verify the hex tag is valid
  if (!\ctype_xdigit($hexTag)) {
    echo "  ERROR: Auth tag is not valid hex: '$hexTag'\n";
    return false;
  }

  // Parse the JSON
  $decoded = \json_decode($jsonPart, true);
  if ($decoded === null) {
    echo "  ERROR: Failed to parse JSON: '$jsonPart'\n";
    return false;
  }

  // Verify class_name
  $actualClass = $decoded['class_name'] ?? null;
  if ($actualClass !== $expected_class) {
    echo "  ERROR: Expected class '$expected_class', got '" . ($actualClass ?? "null") . "'\n";
    return false;
  }

  // Verify function_name
  $actualFunc = $decoded['function_name'] ?? null;
  if ($actualFunc !== $expected_func) {
    echo "  ERROR: Expected function '$expected_func', got '$actualFunc'\n";
    return false;
  }

  // Verify valid_until is present and is a reasonable timestamp
  $validUntil = $decoded['valid_until'] ?? null;
  if ($validUntil === null || !\is_int($validUntil)) {
    echo "  ERROR: Missing or invalid valid_until field\n";
    return false;
  }

  // Check that valid_until is in the future (within 5 minutes)
  $now = \time();
  if ($validUntil <= $now || $validUntil > $now + 301) {
    echo "  ERROR: valid_until should be within 5 minutes from now\n";
    return false;
  }

  return true;
}

<<__EntryPoint>>
function main(): void {
  // Test pack/unpack of namespaced free function
  $cred = namespaced_function();
  $packed = $cred->pack();
  echo "Packed namespaced function credential\n";
  // Namespaced function name includes the namespace prefix
  echo "  Format valid: " . (validate_format($packed, null, 'FunctionCredentialTest\\namespaced_function') ? "yes" : "no") . "\n";

  $unpacked = \FunctionCredential::unpack($packed);
  echo "Unpacked namespaced function:\n";
  echo "  Class: " . ($unpacked->getClassName() ?? "(null)") . "\n";
  echo "  Function: " . $unpacked->getFunctionName() . "\n";
  echo "\n";

  // Test pack/unpack of namespaced static method
  $cred2 = NamespacedClass::static_method();
  $packed2 = $cred2->pack();
  echo "Packed namespaced static method credential\n";
  // Namespaced class name includes the namespace prefix
  echo "  Format valid: " . (validate_format($packed2, 'FunctionCredentialTest\\NamespacedClass', 'static_method') ? "yes" : "no") . "\n";

  $unpacked2 = \FunctionCredential::unpack($packed2);
  echo "Unpacked namespaced static method:\n";
  echo "  Class: " . ($unpacked2->getClassName() ?? "(null)") . "\n";
  echo "  Function: " . $unpacked2->getFunctionName() . "\n";
  echo "\n";

  // Test pack/unpack of namespaced instance method
  $obj = new NamespacedClass();
  $cred3 = $obj->instance_method();
  $packed3 = $cred3->pack();
  echo "Packed namespaced instance method credential\n";
  echo "  Format valid: " . (validate_format($packed3, 'FunctionCredentialTest\\NamespacedClass', 'instance_method') ? "yes" : "no") . "\n";

  $unpacked3 = \FunctionCredential::unpack($packed3);
  echo "Unpacked namespaced instance method:\n";
  echo "  Class: " . ($unpacked3->getClassName() ?? "(null)") . "\n";
  echo "  Function: " . $unpacked3->getFunctionName() . "\n";
  echo "\n";

  // Verify the unpacked credentials match the originals
  echo "Verification:\n";
  echo "  Namespaced function class matches: " . ($cred->getClassName() === $unpacked->getClassName() ? "yes" : "no") . "\n";
  echo "  Namespaced function name matches: " . ($cred->getFunctionName() === $unpacked->getFunctionName() ? "yes" : "no") . "\n";
  echo "  Namespaced static method class matches: " . ($cred2->getClassName() === $unpacked2->getClassName() ? "yes" : "no") . "\n";
  echo "  Namespaced static method name matches: " . ($cred2->getFunctionName() === $unpacked2->getFunctionName() ? "yes" : "no") . "\n";
  echo "  Namespaced instance method class matches: " . ($cred3->getClassName() === $unpacked3->getClassName() ? "yes" : "no") . "\n";
  echo "  Namespaced instance method name matches: " . ($cred3->getFunctionName() === $unpacked3->getFunctionName() ? "yes" : "no") . "\n";
}
