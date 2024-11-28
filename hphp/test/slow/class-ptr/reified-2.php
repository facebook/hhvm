<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function f<reify T>(T $t): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($t);
}

<<__EntryPoint>>
function main(): void {
  f<class<C>>(C::class); // banned by hack
  f<class<C>>(HH\classname_to_class(C::class)); // banned by hack

  f<classname<C>>(nameof C); // banned by hack
  f<classname<C>>(C::class); // banned by hack
  f<classname<C>>(HH\classname_to_class(C::class)); // banned by hack
}
