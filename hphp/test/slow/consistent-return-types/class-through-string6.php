<?hh

// Test that the JIT type computed during irgen reflects the return type/value
// at runtime for methods that inherit string types but may or maynot return
// a class/lazyclass at runtime.
class C {}

enum E: string {
  A = "a";
}

type TStrAlias = string;
type TIntAlias = int;

class Base {
  public function fromClassHint(): string { return "a"; }
  public function fromMixedClass(): string { return "b"; }
  public function fromMixedLazy(): string { return "c"; }
  public function fromAK(): arraykey { return "d"; }
  public function fromEnum(): E { return E::A; }
  public function fromStrAlias(): TStrAlias { return "e"; }
  public function fromIntAlias(): TIntAlias { return 2; }
}

class Derived extends Base {
  public function fromClassHint(): class<C> {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  // Return type is mixed but value at runtime is a class
  public function fromMixedClass(): mixed {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  // Return type is mixed but value at runtime is a lazyclass
  public function fromMixedLazy(): mixed {
    return __hhvm_intrinsics\launder_value(C::class);
  }

  // Has an intersection with the inherited string compatible type but is
  // narrower than the inherited type
  public function fromAK(): int {
    return 1;
  }

  public function fromEnum(): mixed {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  public function fromStrAlias(): mixed {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  // The inherited alias is not string compatible
  public function fromIntAlias(): mixed {
    return __hhvm_intrinsics\launder_value(1);
  }
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(
    (int $errno, string $errstr) ==> {
      if ($errno == E_WARNING) {
        var_dump("Warning: ".$errstr);
      } else if ($errno == E_NOTICE) {
        var_dump("Notice: ".$errstr);
      } else {
        throw new Exception($errstr);
      }
    }
  );

  $o = new Derived();
  var_dump($o->fromClassHint());
  var_dump($o->fromMixedClass());
  var_dump($o->fromMixedLazy());
  var_dump($o->fromAK());
  var_dump($o->fromEnum());
  var_dump($o->fromStrAlias());
  var_dump($o->fromIntAlias());
}
