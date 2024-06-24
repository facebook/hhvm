//// foo.php
<?hh
// package pkg1
type TFoo = int;
interface IFoo<T> {}

//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TBar = string;
interface IBar<T> {}

class Bar1 implements IBar<TFoo> {} // ok
class Bar2 implements IFoo<TBar> {} // error
