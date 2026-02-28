<?hh

namespace N {
  <<__DynamicallyReferenced>> class C {}

  <<__EntryPoint>>
  function main(): void {
    \__hhvm_intrinsics\debug_var_dump_lazy_class(
      // Just checking HackC rewriting works with
      // fully namespace-qualified call
      \HH\classname_to_class("N\C")
    );
  }
}
