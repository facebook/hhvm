<?hh

class SomeClass {
  public static function pathA() {
    return (new SomeClass())->getStackHash();
  }

  public static function pathB() {
    return (new SomeClass())->getStackHash();
  }

  private function getStackHash() {
    return hphp_debug_backtrace_hash();
  }
}

$hash_a = SomeClass::pathA();
$hash_a2 = SomeClass::pathA();
if ($hash_a === $hash_a2) {
  echo "same hash for same path\n";
}

$hash_b = SomeClass::pathB();
if ($hash_a !== $hash_b) {
  echo "different hash for different paths\n";
} else {
  echo "BAD: same hash for different paths ($hash_a)\n";
}
