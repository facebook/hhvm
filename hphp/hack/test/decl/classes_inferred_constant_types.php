<?hh

class C {
  const FOO = -1;
  const BAR = 0;
  const BAZ = +1;
  const QUX = ('s');

  public function f(): void {
    hh_show(C::FOO);
    hh_show(C::BAR);
    hh_show(C::BAZ);
    hh_show(C::QUX);
  }
}
