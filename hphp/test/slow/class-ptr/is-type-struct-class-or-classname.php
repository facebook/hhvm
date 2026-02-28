<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}

<<__EntryPoint>>
function f(): void {
  __hhvm_intrinsics\isTypeStructShallow<class<mixed>>(nameof C);
}
