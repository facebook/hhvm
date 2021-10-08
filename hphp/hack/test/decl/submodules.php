<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
function internal_function_in_parent(): void {}

<<__Module("A.B")>>
function call_internal_functions(): void {
  internal_function_in_child();
  internal_function_in_parent();
}

<<__Module("A.B.C"), __Internal>>
function internal_function_in_child(): void { /* ... */ }

function non_module_call_internal(): void {
  // OK right now, as there are no rules about whether non-modules can call
  // non-internal methods in submodules. This is illegal in, say, C++20 modules.
  call_internal_functions();
}

<<__Module('D')>>
function disjoint_module_call_non_internal(): void {
  // OK right now, as there are no rules about whether disjoint modules can call
  // non-internal methods in submodules. This is illegal in, say, C++20 modules.
  call_internal_functions();
}
