//// Foo.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

enum Foo : int {}
// CHECK-NOT: .fatal
// CHECK-NOT: File "enum.php--Foo.php"

//// Arraykey.php
<?hh
enum Arraykey : int {}
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
enum Bool : int {}
// CHECK: Bool.php
// CHECK: A name is expected here.

//// Callable.php
<?hh
enum Callable : int {}
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
enum Classname : int {}
// CHECK-NOT: .fatal
// CHECK-NOT: File "interface.php--Classname.php"

//// Darray.php
<?hh
enum Darray : int {}
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
enum Dynamic : int {}
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
enum False : int {}
// CHECK: False.php
// CHECK: A name is expected here.

//// Float.php
<?hh
enum Float : int {}
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
enum Int : int {}
// CHECK: Int.php
// CHECK: A name is expected here.

//// Mixed.php
<?hh
enum Mixed : int {}
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
enum Nonnull : int {}
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
enum Noreturn : int {}
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
enum Nothing : int {}
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
enum Null : int {}
// CHECK: Null.php
// CHECK: A name is expected here.

//// Num.php
<?hh
enum Num : int {}
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
enum Parent : int {}
// CHECK: Parent.php
// CHECK: A name is expected here.

//// Resource.php
<?hh
enum Resource : int {}
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
enum Self : int {}
// CHECK: Self.php
// CHECK: A name is expected here.

//// Static.php
<?hh
enum Static : int {}
// CHECK: Static.php
// CHECK: A name is expected here.

//// String.php
<?hh
enum String : int {}
// CHECK: String.php
// CHECK: A name is expected here.

//// This.php
<?hh
enum This : int {}
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
enum True : int {}
// CHECK: True.php
// CHECK: A name is expected here.

//// Varray.php
<?hh
enum Varray : int {}
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
enum Void : int {}
// CHECK: Void.php
// CHECK: A name is expected here.

//// _.php
<?hh
enum _ : int {}
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
