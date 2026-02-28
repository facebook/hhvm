//// Foo.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

newtype Foo = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "newtype.php--Foo.php"

//// Arraykey.php
<?hh
newtype Arraykey = int;
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
newtype Bool = int;
// CHECK: Bool.php
// CHECK: A semicolon `;` is expected here

//// Callable.php
<?hh
newtype Callable = int;
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
newtype Classname = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Classname.php"

//// Darray.php
<?hh
newtype Darray = int;
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
newtype Dynamic = int;
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
newtype False = int;
// CHECK: False.php
// CHECK: A semicolon `;` is expected here

//// Float.php
<?hh
newtype Float = int;
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
newtype Int = int;
// CHECK: Int.php
// CHECK: A semicolon `;` is expected here

//// Mixed.php
<?hh
newtype Mixed = int;
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
newtype Nonnull = int;
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
newtype Noreturn = int;
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
newtype Nothing = int;
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
newtype Null = int;
// CHECK: Null.php
// CHECK: A semicolon `;` is expected here

//// Num.php
<?hh
newtype Num = int;
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
newtype Parent = int;
// CHECK: Parent.php
// CHECK: A semicolon `;` is expected here

//// Resource.php
<?hh
newtype Resource = int;
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
newtype Self = int;
// CHECK: Self.php
// CHECK: A semicolon `;` is expected here

//// Static.php
<?hh
newtype Static = int;
// CHECK: Static.php
// CHECK: A semicolon `;` is expected here

//// String.php
<?hh
newtype String = int;
// CHECK: String.php
// CHECK: A semicolon `;` is expected here

//// This.php
<?hh
newtype This = int;
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
newtype True = int;
// CHECK: True.php
// CHECK: A semicolon `;` is expected here

//// Varray.php
<?hh
newtype Varray = int;
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
newtype Void = int;
// CHECK: Void.php
// CHECK: A semicolon `;` is expected here

//// _.php
<?hh
newtype _ = int;
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
