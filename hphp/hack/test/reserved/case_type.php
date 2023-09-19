//// Foo.php
<?hh
// RUN: %hackc compile -vHack.Lang.AllowUnstableFeatures=true %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

<<file:__EnableUnstableFeatures('case_types')>>
case type Foo = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "case_type.php--Foo.php"

//// Arraykey.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Arraykey = int;
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Bool = int;
// CHECK: Bool.php
// CHECK: A name is expected here

//// Callable.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Callable = int;
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Classname = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Classname.php"

//// Darray.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Darray = int;
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Dynamic = int;
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type False = int;
// CHECK: False.php
// CHECK: A name is expected here

//// Float.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Float = int;
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Int = int;
// CHECK: Int.php
// CHECK: A name is expected here

//// Mixed.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Mixed = int;
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Nonnull = int;
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Noreturn = int;
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Nothing = int;
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Null = int;
// CHECK: Null.php
// CHECK: A name is expected here

//// Num.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Num = int;
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Parent = int;
// CHECK: Parent.php
// CHECK: A name is expected here

//// Resource.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Resource = int;
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Self = int;
// CHECK: Self.php
// CHECK: A name is expected here

//// Static.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Static = int;
// CHECK: Static.php
// CHECK: A name is expected here

//// String.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type String = int;
// CHECK: String.php
// CHECK: A name is expected here

//// This.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type This = int;
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type True = int;
// CHECK: True.php
// CHECK:A name is expected here

//// Varray.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Varray = int;
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type Void = int;
// CHECK: Void.php
// CHECK: A name is expected here

//// _.php
<?hh
<<file:__EnableUnstableFeatures('case_types')>>
case type _ = int;
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
