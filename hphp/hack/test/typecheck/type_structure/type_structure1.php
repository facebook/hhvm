<?hh // strict

class C<T> {
  const type TC1 = this;
}

type MyAlias<T> = C<T>;
type MyAlias2<T> = (T, T);
type MyAlias3<T> = (function(MyAlias2<T>): Vector<T>);

class D extends C<int> {
  const type TC2 = int;
  const type TC3 = Vector<int>;
  const type TC4 = MyAlias<string>;
  const type TC5 = MyAlias3<C<int>>;

  public function test(): void {
    hh_show(type_structure(D::class, 'TC2'));
    hh_show(type_structure(C::class, 'TC1')['classname']);
    hh_show(type_structure(D::class, 'TC1')['classname']);
    hh_show(type_structure(D::class, 'TC3')['classname']);
    hh_show(type_structure(D::class, 'TC4')['classname']);
    hh_show(type_structure(D::class, 'TC5')['classname']);
  }
}
