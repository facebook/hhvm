//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Module("A"), __Internal>>
function internal_function_in_parent(): void {}

//// AB.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A.B')>>

<<__Module("A.B")>>
function call_internal_functions(): void {
  internal_function_in_child();
  internal_function_in_parent();
}

//// ABC.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A.B.C')>>

<<__Module("A.B.C"), __Internal>>
function internal_function_in_child(): void { /* ... */ }

//// D.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('D')>>

<<__Module('D')>>
function disjoint_module_call_non_internal(): void {
  // OK right now, as there are no rules about whether disjoint modules can call
  // non-internal methods in submodules. This is illegal in, say, C++20 modules.
  call_internal_functions();
}

//// no-module.php
<?hh

function non_module_call_internal(): void {
  // OK right now, as there are no rules about whether non-modules can call
  // non-internal methods in submodules. This is illegal in, say, C++20 modules.
  call_internal_functions();
}
