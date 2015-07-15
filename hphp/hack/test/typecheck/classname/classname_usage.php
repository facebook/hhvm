<?hh // strict

trait Tr {
  public static function bar(): string {
    return 'via trait'.__CLASS__;
  }
}
interface I {
  public function foo(): int;
  public static function bar(): string;
}
class C1 implements I {
  public function foo(): int {
    return 1;
  }
  use Tr;
}
class C2 implements I {
  public function foo(): int {
    return 2;
  }
  use Tr;
}
class C_NotI {
  // not I
  public function foo(): int {
    return 3;
  }
}

function call_foo<T as I>(Vector<T> $vals): int {
  $ret = 0;
  foreach ($vals as $v) {
    $ret += $v->foo();
  }
  return $ret;
}

function make_vector<T>(classname<T> $classname): Vector<T> {
  $ret = Vector {};
  $ret[] = factory($classname);
  $ret[] = factory($classname);
  return $ret;
}

function factory<T>(classname<T> $classname): T {
  // UNSAFE_BLOCK
}

function foo(): void {
  make_vector(C1::class);
  make_vector(Tr::class);
  make_vector(I::class);

  call_foo(make_vector(C1::class));
  call_foo(make_vector(C2::class));
  // call_foo(make_vector(C_NotI::class)); // error, C_NotI is not an I

  list_children_of_I();
}

function list_children_of_I(): ConstVector<classname<I>> {
  $v = Vector {C1::class};
  $v[] = C2::class;
  return $v;
}

abstract class Super {
  public abstract function nameOfISubclass(): classname<I>;
}

class Sub extends Super {
  public function nameOfISubclass(): classname<C1> {
    return C1::class;
  }
}

function classname_polymorphism1(classname<I> $name): string {
  $ret = $name::bar();
  hh_show($ret);
  return $ret;
}

function classname_polymorphism2<T as I>(classname<T> $name): string {
  hh_show($name);
  $ret = $name::bar();
  hh_show($ret);
  return $ret;
}
