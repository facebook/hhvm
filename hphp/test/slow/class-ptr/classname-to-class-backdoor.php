<?hh

<<__DynamicallyReferenced>>
class C { public static function f(): void { echo "f\n"; } }

class D { public static function f(): void { echo "f\n"; } }


<<__EntryPoint>>
function main(): void {
  $static_c = nameof C;
  $dynamic_c = __hhvm_intrinsics\launder_value($static_c)."";
  $static_d = nameof D;
  $dynamic_d = __hhvm_intrinsics\launder_value($static_d)."";

  HH\classname_to_class($static_c) |> $$::f();
  HH\classname_to_class($dynamic_c) |> $$::f();
  HH\classname_to_class($static_d) |> $$::f(); // log
  HH\classname_to_class($dynamic_d) |> $$::f(); // log

  HH\classname_to_class($static_c, 'cause_a_sev') |> $$::f();
  HH\classname_to_class($dynamic_c, 'cause_a_sev') |> $$::f();
  HH\classname_to_class($static_d, 'cause_a_sev') |> $$::f();
  HH\classname_to_class($dynamic_d, 'cause_a_sev') |> $$::f();

  HH\classname_to_class($static_c, "cause_a_sev") |> $$::f();
  HH\classname_to_class($dynamic_c, "cause_a_sev") |> $$::f();
  HH\classname_to_class($static_d, "cause_a_sev") |> $$::f();
  HH\classname_to_class($dynamic_d, "cause_a_sev") |> $$::f();
}
