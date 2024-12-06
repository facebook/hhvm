<?hh

class A {
  public static function f(): void {
    $x = nameof static;
    echo "called on $x\n";
  }
}

class C extends A {}

function class_from_obj_case(): void {
  // alternate to class pointer case
  $obj = new C();
  $obj::f();
  $c = HH\get_class_from_object($obj);
  $a = HH\get_parent_class_from_class($c);
  $a::f();
}

function class_ptr_case(): void {
  $c = HH\classname_to_class(C::class);
  $a = HH\get_parent_class_from_class($c);
  $a::f();
}

function lazy_class_case(): void {
  $a = HH\get_parent_class_from_class(C::class);
  $a::f();
}

function string_case(): void {
  // logs a string going through a class<T> type hint
  $a = HH\get_parent_class_from_class(nameof C);
  $a::f();

  // non-persistent string
  $aa = "C".__hhvm_intrinsics\launder_value("");
  $a = HH\get_parent_class_from_class($aa);
  $a::f();
}

function null_case($x): void {
  $n = HH\get_parent_class_from_class($x);
  invariant($n === null, '');
  invariant($n !== false, '');

  // compare to \get_parent_class which returns false
  $f = get_parent_class($x);
  invariant($f === false, '');
  invariant($f !== null, '');
}

<<__EntryPoint>>
function main(): void {
  class_from_obj_case();
  class_ptr_case();
  lazy_class_case();
  string_case();

  // no parent case
  null_case(HH\classname_to_class(A::class));
  null_case(A::class);
  null_case(nameof A); // logs
  null_case("A".__hhvm_intrinsics\launder_value("")); // logs

  // nonexistent class case
  null_case(X::class);
  null_case(nameof X); // logs
  null_case("X".__hhvm_intrinsics\launder_value("")); // logs
}
