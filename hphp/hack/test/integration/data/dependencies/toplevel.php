<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}

class A0 {}

class A extends A0 implements I1 {}

class B implements I1 {}

class C extends A {}

class D<Tfirst, Tsecond> extends B {}

class E<T> extends D<T, int> {}

type Complex = shape('first' => int, 'second' => B);

newtype Point = shape('x' => int, 'y' => int);

function generic<T>(): int {
  return 1;
}

function generic_with_bound<T as arraykey>(T $x): keyset<T> {
  return keyset[$x];
}

function g() : void {
  $b = new B();
}

function shallow_toplevel(C $c): void  {
  g();
}

function with_generics<Tfirst, Tsecond>(D<Tfirst, Tsecond> $d, E<Tfirst> $e): int {
  return generic<C>();
}

function with_generics_with_bounds(int $x): keyset<int> {
  return generic_with_bound($x);
}

function with_typedefs(Complex $c, shape('x' => int, 'y' => C) $pair) : Point {
  return shape('x' => $pair['x'], 'y' => $c['first']);
}

function with_defaults(int $arg = 42, float $argf = 4.2): void {
}

function call_defaulted(int $arg): void {
  with_defaults($arg);
  with_defaults();
}

function with_default_and_variadic(mixed $x, ?string $y = null, mixed ...$z): void {}

function call_with_default_and_variadic(string $s): void {
  with_default_and_variadic(42);
  with_default_and_variadic(42, 'meaning of life');
  with_default_and_variadic(42, '%s', $s);
}

function nonexistent_dependency(BogusType $arg): void {}

function builtin_argument_types(Exception $e, keyset<string> $k): void {}

function recursive_function(int $n): int {
  if ($n <= 0) {
    return 0;
  }
  return $n + recursive_function($n - 1);
}

class WithRecursiveMethods {
  public function recursive_instance(): void {
    $this->recursive_instance();
  }
  public static function recursive_static(): void {
    WithRecursiveMethods::recursive_static();
  }
}

function with_mapped_namespace(): void {
  PHP\ini_set('foo', 42);
}

function with_built_in_constant(): int {
  return PHP_INT_MAX;
}

function reactive(mixed $x = null): void {}

function call_reactive(): void {
  reactive();
}

class Fred {}

class Thud {
  public int $n;
  public function __construct(Fred $_) {
    $this->n = 42;
  }
}

function with_constructor_dependency(Thud $x): int {
  return $x->n;
}

function with_newtype_with_bound(dict<N, mixed> $_): void {}

newtype M as N = nothing;

function with_newtype_with_newtype_bound(M $_): void {}

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

function with_unsafe_type_hh_fixme(UNSAFE_TYPE_HH_FIXME $x): int {
  return $x;
}

type Option<T> = Id<?T>;
type Id<T> = T;

class WithTypeAliasHint {
  private Option<int> $x = null;

  public function getX(): Option<int> {
    return $this->x;
  }
}

type TydefWithFun<T> = (
  (function(int, ?T): void),
  (function(int, float...):void)
);

function function_in_typedef<T>(TydefWithFun<T> $y):void {}

type TydefWithCapabilities<T> = (
  (function(): void),
  (function()[]: void),
  (function()[write_props,rx]: void),
);

function contexts_in_typedef<T>(TydefWithCapabilities<T> $y):void {}

function with_argument_dependent_context_callee(
  (function()[_]: void) $f,
)[write_props, ctx $f]: void {
  $f();
}

function with_argument_dependent_context()[ defaults, zoned]: void {
  with_argument_dependent_context_callee(()[defaults] ==> {
    echo "write";
  });
}

class Contextual {
  public static function with_argument_dependent_context(
    (function()[_]: void) $f,
  )[write_props, ctx $f]: void {
    $f();
  }
}

class WithContextConstant {
  const ctx C = [defaults];
  public function has_io()[self::C]: void {
    echo "I have IO!";
  }
}

function with_optional_argument_dependent_context_callee(
  ?(function()[_]: void) $f1,
)[ctx $f1]: void {
  if ($f1 is nonnull) {
    $f1();
  }
}

function with_optional_argument_dependent_context(): void {
  with_optional_argument_dependent_context_callee(null);
}

function my_keys<Tk as arraykey, Tv>(
   KeyedTraversable<Tk, Tv> $traversable,
)[]: keyset<Tk> {
  $result = keyset[];
  foreach ($traversable as $key => $_) {
    $result[] = $key;
  }
  return $result;
}

function with_expr_in_user_attrs(): void {
  $_ = my_keys(vec[]);
}

<<MyUserAttr(SimpleClass::class)>>
type WithClassNameInAttr= int;

<<MyUserAttr('blah \' blah blah')>>
type WithEscapedCharInAttr= int;

<<MyUserAttr('blah')>>
type TransparentWithUserAttr = int;

<<MyUserAttr('blah')>>
newtype OpaqueWithUserAttr = int;

<<MyUserAttr('blah')>>
enum EnumWithUserAttr: int as int {
  Blah = 0;
}

function enum_with_user_attr(EnumWithUserAttr $x): void {}

function transparent_with_user_attr(TransparentWithUserAttr $x): void {}

function opaque_with_user_attr(OpaqueWithUserAttr $x): void {}

function with_escaped_char_in_attr(WithEscapedCharInAttr $_): void {}

function with_class_name_in_attr(WithClassNameInAttr $_): void {}

<<MyUserAttr('blah')>>
class WithUserAttr {
  public function foo(): void {}
}

class WithMethodWithUserAttr {
  <<MyUserAttr('blah')>>
  public function foo(): void {}
}

class WithTypeConstantWithUserAttr {
  <<MyUserAttr('blah')>>
  const type T = int;
  public function foo(): self::T {
    return 1;
  }
}

class WithStaticPropWithUserAttr {
  <<MyUserAttr('blah')>> public static int $i = 0;
  public function foo(): int {
    return self::$i;
  }
}

class WithPropWithUserAttr {
  <<MyUserAttr('blah')>> public int $i = 0;
  public function foo(): int {
    return $this->i;
  }
}

class WithConstrPropWithUserAttr {
  public function __construct(<<MyUserAttr('blah')>> private int $i){}
}

function with_constr_prop_with_user_attr():void {
  $_ = new WithConstrPropWithUserAttr(2);
}

<<MyUserAttr('blah')>>
function with_user_attr(): void {}

function with_param_with_user_attr(<<MyUserAttr('blah')>> int $s): void {}

function with_tparam_with_user_attr<<<MyUserAttr('blah')>> T>(T $x): void {}

final class MyUserAttr
  implements
    HH\ClassAttribute,
    HH\MethodAttribute,
    HH\TypeAliasAttribute,
    HH\EnumAttribute,
    HH\FunctionAttribute,
    HH\InstancePropertyAttribute,
    HH\StaticPropertyAttribute,
    HH\ParameterAttribute,
    HH\TypeParameterAttribute,
    HH\TypeConstantAttribute {
  public function __construct(string $first, string ...$remainder)[] {}
}


interface IC {
    public function foo(): void;
}

class DC implements IC {
    public function foo(): void {}
}

class WithWhereConstraint<T> {
    public function __construct(T $x) where T as IC {
        $x->foo();
    }
}

function with_where_constraint(): void {
    $z = new WithWhereConstraint(new DC());
}



function with_open_shape(shape(...) $x): void {}

function with_tparam_constraint(WithTparamConstraint<IAsConstraint> $_) : void {}

function with_prop_in_construct() : void {
  $x = new WithPropInConstruct(1);
}
