//// Foo.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory %s --no-builtins 2>&1 || true; } | FileCheck %s

interface Foo {}
// CHECK-NOT: .fatal
// CHECK-NOT: File "interface.php--Foo.php"

//// Arraykey.php
<?hh
interface Arraykey {}
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
interface Bool {}
// CHECK: Bool.php
// CHECK: Cannot use `Bool` as a type name as it is reserved

//// Callable.php
<?hh
interface Callable {}
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
interface Classname {}
// CHECK-NOT: .fatal
// CHECK-NOT: File "interface.php--Classname.php"

//// Darray.php
<?hh
interface Darray {}
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
interface Dynamic {}
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
interface False {}
// CHECK: False.php
// CHECK: Cannot use `False` as a type name as it is reserved

//// Float.php
<?hh
interface Float {}
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
interface Int {}
// CHECK: Int.php
// CHECK: Cannot use `Int` as a type name as it is reserved

//// Mixed.php
<?hh
interface Mixed {}
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
interface Nonnull {}
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
interface Noreturn {}
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
interface Nothing {}
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
interface Null {}
// CHECK: Null.php
// CHECK: Cannot use `Null` as a type name as it is reserved

//// Num.php
<?hh
interface Num {}
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
interface Parent {}
// CHECK: Parent.php
// CHECK: Cannot use `Parent` as a type name as it is reserved

//// Resource.php
<?hh
interface Resource {}
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
interface Self {}
// CHECK: Self.php
// CHECK: Cannot use `Self` as a type name as it is reserved

//// Static.php
<?hh
interface Static {}
// CHECK: Static.php
// CHECK: A name is expected here

//// String.php
<?hh
interface String {}
// CHECK: String.php
// CHECK: Cannot use `String` as a type name as it is reserved

//// This.php
<?hh
interface This {}
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
interface True {}
// CHECK: True.php
// CHECK: Cannot use `True` as a type name as it is reserved

//// Varray.php
<?hh
interface Varray {}
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
interface Void {}
// CHECK: Void.php
// CHECK: Cannot use `Void` as a type name as it is reserved

//// _.php
<?hh
interface _ {}
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
