<?hh

class Bar {
  const type BarBool = bool;
}

class Foo<TType> {
  const type FooEmptyShape = shape();
  const type FooShape = shape('a' => bool, 'b' => int);
  const type FooBool = bool;
  const type FooMixed = ?mixed;
}

class ReifiedC<reify T> {}

function f<reify T>(T $x): ReifiedC<T> {
  return new ReifiedC<T>();
}

newtype FooInt = int;
newtype FooString = string;
newtype FooTypevar<F> = F;
newtype FooTypevarTypes = FooTypevar<int>;
newtype FooClass = Foo<bool>;
newtype FooAny = ?AnyArray<string, bool>;
newtype FooTuple = (FooString, mixed);

newtype FooSoftInt = @~int;
newtype FooLikeInt = ~@int;

type ComplicatedType<T> = shape(
  'a' => int,
  ?'b' => vec<T>,
  ?'c' => vec<shape(
    'd' => ReifiedC<bool>,
    ?'e' => FooAny,
    ...
  )>,
  ...
);
type FooComplicatedType = ComplicatedType<int>;

<<__EntryPoint>>
function main(): void {
  $foo = __hhvm_intrinsics\launder_value(Foo::class);

  $ts = type_structure($foo, 'FooEmptyShape');
  var_dump($ts['kind']);
  var_dump($ts['fields']);

  $ts = type_structure($foo, 'FooShape');
  var_dump($ts);
  var_dump($ts['kind']);
  var_dump($ts['fields']);
  var_dump($ts['allows_unknown_fields'] ?? 'field exists but is false');
  var_dump($ts['classname'] ?? 'correct field name but not on this kind');

  $ts = type_structure($foo, 'FooBool');
  var_dump($ts['kind']);
  var_dump($ts['nullable'] ?? 'field exists but is false');

  $ts = type_structure($foo, 'FooMixed');
  var_dump($ts['kind']);
  var_dump($ts['nullable']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooInt'), null);
  var_dump($ts['alias']);
  var_dump($ts['kind']);
  var_dump($ts['opaque']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooTypevarTypes'), null);
  var_dump($ts);
  var_dump($ts['alias']);
  var_dump($ts['typevar_types']);
  var_dump($ts['kind']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooClass'), null);
  var_dump($ts);
  var_dump($ts['kind']);
  var_dump($ts['classname']);
  var_dump($ts['generic_types']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooAny'), null);
  var_dump($ts['kind']);
  var_dump($ts['generic_types']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooTuple'), null);
  var_dump($ts['alias']);
  var_dump($ts['kind']);
  var_dump($ts['elem_types']);

  $ts = type_structure('Bar', 'BarBool');
  var_dump($ts['kind']);
  $ts = type_structure('FooInt', null);
  var_dump($ts['kind']);
  $ts = type_structure('FooString', null);
  var_dump($ts['opaque']);

  $reified = HH\ReifiedGenerics\get_type_structure<ReifiedC<int>>();
  var_dump($reified['kind']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooComplicatedType'));
  var_dump($ts['kind']);
  var_dump($ts['typevar_types']);

  var_dump(type_structure(__hhvm_intrinsics\launder_value('FooSoftInt'), null));
  var_dump(type_structure(__hhvm_intrinsics\launder_value('FooLikeInt'), null));
}
