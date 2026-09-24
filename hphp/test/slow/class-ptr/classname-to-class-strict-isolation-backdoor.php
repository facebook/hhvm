<?hh

<<__DynamicallyReferenced>>
class Allowed {
  public static function f(): void {
    echo "allowed\n";
  }
}

class Missing {
  public static function f(): void {
    echo "missing\n";
  }
}

<<__EntryPoint>>
function main(): void {
  HH\classname_to_class_strict_isolation_backdoor(nameof Allowed) |> $$::f();
  HH\classname_to_class_strict_isolation_backdoor(nameof Missing) |> $$::f();
}
