<?hh // strict

class C<T> {}

type MyAlias<T> = C<T>;
type MyAlias2<T> = (T, T);
type MyAlias3<T> = (function(T): Vector<T>);

class D extends C<int> {
  public function test(
    TypeStructure<int> $a,
    TypeStructure<Vector<int>> $b,
    TypeStructure<MyAlias<string>> $c,
    TypeStructure<MyAlias2<bool>> $d,
    TypeStructure<MyAlias3<C<int>>> $e,
    TypeStructure<this> $f,
  ): void {
    hh_show($a['kind']);
    hh_show($a['nullable']);
    hh_show($b['classname']);
    hh_show($c['classname']);
    hh_show($d['classname']);
    hh_show($e['classname']);
    hh_show($f['classname']);
  }
}
