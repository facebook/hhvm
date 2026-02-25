//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TFoo = int;
interface IFoo<T> {}
class Foo {}

//// bar.php
<?hh
// package pkg1
type TBar = string;
interface IBar<T> {}
class Bar<T> {
  public static function staticMethod<Tu>(): void {}
  public function method<Tu>(): void {}
}
function bar<T>(): void {}
trait TrBar<T> {}

// --- Declaration-level generics: should NOT error ---

// implements with cross-package generic type arg
class Bar1 implements IBar<TFoo> {}

// extends with cross-package generic type arg
class Bar5 extends Bar<TFoo> {}

// trait use with cross-package generic type arg
class Bar6 { use TrBar<TFoo>; }

// type alias with cross-package generic type arg
type TAlias = Bar<TFoo>;

// function parameter type hint with cross-package generic
function param_hint(Bar<TFoo> $_): void {}

// function return type hint with cross-package generic
function return_hint(): Bar<TFoo> {
  return new Bar();
}

// property type with cross-package generic
class Bar7 {
  public ?Bar<TFoo> $prop = null;
}

// Hack array types with cross-package generic type arg
function vec_hint(vec<TFoo> $_): void {}
function dict_hint(dict<string, TFoo> $_): void {}
function keyset_hint(keyset<TFoo> $_): void {}
type TVecAlias = vec<TFoo>;
type TDictAlias = dict<string, TFoo>;

// type parameter constraint (not a type arg)
class Bar3<T as TFoo> {}
class Bar4<T as Foo> {}

// --- Symbol-level errors (not generics) ---

// IFoo is from pkg2, so accessing the symbol itself errors
class Bar2 implements IFoo<TBar> {} // error: IFoo is cross-package

// Collection literal explicit targs: should NOT error
function collection_literals(): void {
  $_ = vec<Foo>[];
  $_ = dict<string, Foo>[];
  $_ = keyset<TFoo>[];
}

// --- Explicit targs in calls/new: should error ---

function test(): void {
  $_ = new Bar<Foo>(); // error: explicit targ on new
  bar<Foo>(); // error: explicit targ on function call
  Bar::staticMethod<Foo>(); // error: explicit targ on static method call
  $bar = new Bar();
  $bar->method<Foo>(); // error: explicit targ on instance method call
}
