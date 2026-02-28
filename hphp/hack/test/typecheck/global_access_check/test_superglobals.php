<?hh

// This is to mimic the super global class "GlobalENV" in www/flib/core/superglobals/GlobalENV.php
class GlobalENV {
  public static function get(arraykey $key): mixed {
    return "dummy_value";
  }

  public static function set(arraykey $key, mixed $value): void {
  }
}

// This is to mimic the super global class "GlobalVARIABLES" in www/flib/core/superglobals/GlobalVARIABLES.php
class GlobalVARIABLES {
  public static function get(string $key): mixed {
    return true;
  }

  public static function set(string $key, mixed $value): void {
  }
}

function call_mixed(mixed $x): void {}

class Test {
  public function test_method_call(): void {
    $a = GlobalENV::get("key1"); // global read
    GlobalENV::set("key2", $a); // global write
    call_mixed(GlobalENV::get("key3")); // global read

    $b = GlobalVARIABLES::get('TEST'); // global read
    GlobalVARIABLES::set('Test', false); // global write
    $c = GlobalVARIABLES::get('TEST') ? "a" : "b"; // global read

    $d = 'TEST';
    $e = GlobalVARIABLES::get($d); // global read
    GlobalVARIABLES::set($d, false); // global write
  }
}
