<?hh

type Tc = class<C>;
type Tcn = classname<C>;

class C {
  const type Tc = class<C>;
  const type Tcn = classname<C>;

  public function m(): void {
    f<this::Tc>();
    f<this::Tcn>();
  }
}

function f<reify T>(): void {}

function g(): void {
  f<class<C>>();
  f<classname<C>>();

  f<C::Tc>();
  f<C::Tcn>();

  f<Tc>();
  f<Tcn>();
}
