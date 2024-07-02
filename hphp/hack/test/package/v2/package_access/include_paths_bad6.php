//// foo.php
<?hh
// package pkg1
type TFoo = int;
interface IFoo<T> {}
class Foo {}

//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TBar = string;
interface IBar<T> {}
class Bar<T> {}
function bar<T>(): void {}

class Bar1 implements IBar<TFoo> {} // ok
class Bar2 implements IFoo<TBar> {} // error
class Bar3<T as TFoo> {} // ok
class Bar4<T as Foo> {} // ok

function test(mixed $x): void {
    // both ok
    $_ = new Bar<TFoo>();
    bar<TFoo>();
}
