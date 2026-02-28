<?hh

class SomeClass {
  public static function pathA() :mixed{
    return (new SomeClass())->getStackHash();
  }

  public static function pathB() :mixed{
    return (new SomeClass())->getStackHash();
  }

  public static function pathC(string $metadata, int $options) :mixed{
    HH\set_frame_metadata($metadata);
    return (new SomeClass())->getStackHash($options);
  }

  private function getStackHash(int $options = 0) :mixed{
    return hphp_debug_backtrace_hash($options);
  }
}


<<__EntryPoint>>
function main_debug_backtrace_hash() :mixed{
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

$hash_c = SomeClass::pathC("meta1", 0);
$hash_d = SomeClass::pathC("meta1", DEBUG_BACKTRACE_HASH_CONSIDER_METADATA);
$hash_e = SomeClass::pathC("meta2", DEBUG_BACKTRACE_HASH_CONSIDER_METADATA);
$hash_f = SomeClass::pathC("meta3", 0);

if ($hash_c === $hash_f) {
  echo "metadata is ignored with default options\n";
} else {
  echo "BAD: metadata is not ignored with default options\n";
}

if ($hash_d !== $hash_e && $hash_d !== $hash_c) {
  echo "metadata is considered correctly\n";
} else {
  echo "BAD: metadata is not considered correctly\n";
}

}
