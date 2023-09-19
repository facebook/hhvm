//// mm.php
<?hh
// RUN: %hackc compile %s | FileCheck %s
// RUN: { %hh_single_type_check --error-format plain --custom-hhi-path %empty_directory --no-builtins %s 2>&1 || true; } | FileCheck %s

new module mm {}

//// Foo.php
<?hh
module mm;
module newtype Foo as int = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "module_newtype.php--Foo.php"

//// Arraykey.php
<?hh
module mm;
module newtype Arraykey as int = int;
// CHECK: Arraykey.php
// CHECK: Cannot use `Arraykey` as a type name as it is reserved

//// Bool.php
<?hh
module mm;
module newtype Bool as int = int;
// CHECK: Bool.php
// CHECK: Cannot use `Bool` as a type name as it is reserved

//// Callable.php
<?hh
module mm;
module newtype Callable as int = int;
// CHECK: Callable.php
// CHECK: Cannot use `Callable` as a type name as it is reserved

//// Classname.php
<?hh
module mm;
module newtype Classname as int = int;
// CHECK-NOT: .fatal
// CHECK-NOT: File "type.php--Classname.php"

//// Darray.php
<?hh
module mm;
module newtype Darray as int = int;
// CHECK: Darray.php
// CHECK: Cannot use `Darray` as a type name as it is reserved

//// Dynamic.php
<?hh
module mm;
module newtype Dynamic as int = int;
// CHECK: Dynamic.php
// CHECK: Cannot use `Dynamic` as a type name as it is reserved

//// False.php
<?hh
module mm;
module newtype False as int = int;
// CHECK: False.php
// CHECK: Cannot use `False` as a type name as it is reserved

//// Float.php
<?hh
module mm;
module newtype Float as int = int;
// CHECK: Float.php
// CHECK: Cannot use `Float` as a type name as it is reserved

//// Int.php
<?hh
module mm;
module newtype Int as int = int;
// CHECK: Int.php
// CHECK: Cannot use `Int` as a type name as it is reserved

//// Mixed.php
<?hh
module mm;
module newtype Mixed as int = int;
// CHECK: Mixed.php
// CHECK: Cannot use `Mixed` as a type name as it is reserved

//// Nonnull.php
<?hh
module mm;
module newtype Nonnull as int = int;
// CHECK: Nonnull.php
// CHECK: Cannot use `Nonnull` as a type name as it is reserved

//// Noreturn.php
<?hh
module mm;
module newtype Noreturn as int = int;
// CHECK: Noreturn.php
// CHECK: Cannot use `Noreturn` as a type name as it is reserved

//// Nothing.php
<?hh
module mm;
module newtype Nothing as int = int;
// CHECK: Nothing.php
// CHECK: Cannot use `Nothing` as a type name as it is reserved

//// Null.php
<?hh
module mm;
module newtype Null as int = int;
// CHECK: Null.php
// CHECK: Cannot use `Null` as a type name as it is reserved

//// Num.php
<?hh
module mm;
module newtype Num as int = int;
// CHECK: Num.php
// CHECK: Cannot use `Num` as a type name as it is reserved

//// Parent.php
<?hh
module mm;
module newtype Parent as int = int;
// CHECK: Parent.php
// CHECK: Cannot use `Parent` as a type name as it is reserved

//// Resource.php
<?hh
module mm;
module newtype Resource as int = int;
// CHECK: Resource.php
// CHECK: Cannot use `Resource` as a type name as it is reserved

//// Self.php
<?hh
module mm;
module newtype Self as int = int;
// CHECK: Self.php
// CHECK: Cannot use `Self` as a type name as it is reserved

//// Static.php
<?hh
module mm;
module newtype Static as int = int;
// CHECK: Static.php
// CHECK: A name is expected here

//// String.php
<?hh
module mm;
module newtype String as int = int;
// CHECK: String.php
// CHECK: Cannot use `String` as a type name as it is reserved

//// This.php
<?hh
module mm;
module newtype This as int = int;
// CHECK: This.php
// CHECK: Cannot use `This` as a type name as it is reserved

//// True.php
<?hh
module mm;
module newtype True as int = int;
// CHECK: True.php
// CHECK: Cannot use `True` as a type name as it is reserved

//// Varray.php
<?hh
module mm;
module newtype Varray as int = int;
// CHECK: Varray.php
// CHECK: Cannot use `Varray` as a type name as it is reserved

//// Void.php
<?hh
module mm;
module newtype Void as int = int;
// CHECK: Void.php
// CHECK: Cannot use `Void` as a type name as it is reserved

//// _.php
<?hh
module mm;
module newtype _ as int = int;
// CHECK: _.php
// CHECK: Cannot use `_` as a type name as it is reserved
