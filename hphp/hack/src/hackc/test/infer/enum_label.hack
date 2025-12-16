// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: "type E "
// CHECK: type E extends HH::BuiltinEnumClass = .kind="class" {
// CHECK: }

// TEST-CHECK-BAL: define E$static.__factory
// CHECK: define E$static.__factory($this: *E$static) : *E {
// CHECK: #b0:
// CHECK:   n0 = __sil_allocate(<E>)
// CHECK:   n1 = E._86pinit(n0)
// CHECK:   ret n0
// CHECK: }

// TEST-CHECK-BAL: define E$static._86constinit
// CHECK: define E$static._86constinit($this: .notnull *E$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *E$static = load &$this
// CHECK:   n1 = HH::BuiltinEnumClass$static._86constinit(n0)
// CHECK:   n2 = $builtins.hhbc_class_get_c($builtins.hack_string("E"))
// CHECK:   n3 = $builtins.hack_set_static_prop($builtins.hack_string("E"), $builtins.hack_string("A"), $builtins.hack_int(42))
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define E._86pinit
// CHECK: define E._86pinit($this: .notnull *E) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *E = load &$this
// CHECK:   n1 = HH::BuiltinEnumClass._86pinit(n0)
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define E$static._86sinit
// CHECK: define E$static._86sinit($this: .notnull *E$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *E$static = load &$this
// CHECK:   n1 = HH::BuiltinEnumClass$static._86sinit(n0)
// CHECK:   n2: *E$static = load &$this
// CHECK:   n3 = E$static._86constinit(n2)
// CHECK:   n4 = $builtins.hhbc_class_get_c($builtins.hack_string("E"))
// CHECK:   ret null
// CHECK: }

enum class E: int {
  int A = 42;
}

// TEST-CHECK-BAL: define $root.foo
// CHECK: define $root.foo($this: *void, $label: *HH::EnumClass::Label) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("HH\\EnumClass\\Label"), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("E")), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$label
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 7
// CHECK:   n3: *HackMixed = load &$label
// CHECK: // .column 7
// CHECK:   n4 = $builtins.hhbc_cmp_same(n3, $builtins.hack_enum_label())
// CHECK: // .column 7
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 7
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK: // .column 7
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK: // .column 20
// CHECK:   n5: *HackMixed = load &$label
// CHECK: // .column 10
// CHECK:   n6 = __sil_lazy_class_initialize(<E>)
// CHECK:   n7 = E$static.nameOf(n6, n5)
// CHECK: // .column 10
// CHECK:   n8 = $builtins.hhbc_concat(n7, $builtins.hack_string("\n"))
// CHECK: // .column 5
// CHECK:   n9 = $builtins.hhbc_print(n8)
// CHECK: // .column 5
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function foo(\HH\EnumClass\Label<E, int> $label): void {
  if ($label === E#A) {
    echo E::nameOf($label) . "\n";
  }
}
