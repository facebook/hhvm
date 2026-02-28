//// defaults.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

namespace HH\Contexts {
  type defaults = (I);
  interface I {}
}

//// Foo.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Foo as [defaults];
// CHECK-NOT: .fatal
// CHECK-NOT: File "newctx.php--Foo.php"

//// Arraykey.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Arraykey as [defaults];
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Bool as [defaults];
// CHECK: Bool.php
// CHECK: A name is expected here

//// Callable.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Callable as [defaults];
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Classname as [defaults];
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Classname.php"

//// Darray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Darray as [defaults];
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Dynamic as [defaults];
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx False as [defaults];
// CHECK: False.php
// CHECK: A name is expected here

//// Float.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Float as [defaults];
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Int as [defaults];
// CHECK: Int.php
// CHECK: A name is expected here

//// Mixed.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Mixed as [defaults];
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Nonnull as [defaults];
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Noreturn as [defaults];
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Nothing as [defaults];
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Null as [defaults];
// CHECK: Null.php
// CHECK: A name is expected here

//// Num.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Num as [defaults];
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Parent as [defaults];
// CHECK: Parent.php
// CHECK: A name is expected here

//// Resource.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Resource as [defaults];
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Self as [defaults];
// CHECK: Self.php
// CHECK: A name is expected here

//// Static.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Static as [defaults];
// CHECK: Static.php
// CHECK: A name is expected here

//// String.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx String as [defaults];
// CHECK: String.php
// CHECK: A name is expected here

//// This.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx This as [defaults];
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx True as [defaults];
// CHECK: True.php
// CHECK: A name is expected here

//// Varray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Varray as [defaults];
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Void as [defaults];
// CHECK: Void.php
// CHECK: A name is expected here

//// _.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx _ as [defaults];
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
