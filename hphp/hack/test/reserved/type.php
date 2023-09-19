//// Foo.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

type Foo = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Foo.php"

//// Arraykey.php
<?hh
type Arraykey = int;
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
type Bool = int;
// CHECK: Bool.php
// CHECK: A semicolon `;` is expected here

//// Callable.php
<?hh
type Callable = int;
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
type Classname = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Classname.php"

//// Darray.php
<?hh
type Darray = int;
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
type Dynamic = int;
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
type False = int;
// CHECK: False.php
// CHECK: A semicolon `;` is expected here

//// Float.php
<?hh
type Float = int;
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
type Int = int;
// CHECK: Int.php
// CHECK: A semicolon `;` is expected here

//// Mixed.php
<?hh
type Mixed = int;
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
type Nonnull = int;
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
type Noreturn = int;
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
type Nothing = int;
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
type Null = int;
// CHECK: Null.php
// CHECK: A semicolon `;` is expected here

//// Num.php
<?hh
type Num = int;
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
type Parent = int;
// CHECK: Parent.php
// CHECK: A semicolon `;` is expected here

//// Resource.php
<?hh
type Resource = int;
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
type Self = int;
// CHECK: Self.php
// CHECK: A semicolon `;` is expected here

//// Static.php
<?hh
type Static = int;
// CHECK: Static.php
// CHECK: A semicolon `;` is expected here

//// String.php
<?hh
type String = int;
// CHECK: String.php
// CHECK: A semicolon `;` is expected here

//// This.php
<?hh
type This = int;
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
type True = int;
// CHECK: True.php
// CHECK: A semicolon `;` is expected here

//// Varray.php
<?hh
type Varray = int;
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
type Void = int;
// CHECK: Void.php
// CHECK: A semicolon `;` is expected here

//// _.php
<?hh
type _ = int;
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
