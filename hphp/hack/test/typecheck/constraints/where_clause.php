<?hh

class Foo {
}

class Bar<+Tbar> extends Foo {
  public function barMethod(): this {
    return $this;
  }
}

class Baz<+Tbaz> extends Bar<Tbaz> {
  public function bazMethod<Tcorge>(Tcorge $x): Tcorge where
    Tcorge as Bar<Tcorge>,
  {
    return $x->barMethod();
  }
}

class Qux extends Baz<Qux> {
  <<__Override>>
  public function bazMethod<Tcorge>(Tcorge $x): Tcorge where
    Tcorge as Bar<Tcorge>,
  {
    return $x->barMethod();
  }
}

class WhereClauseTest {
  const where = 'where is not a keyword everywhere';
  public function callBazMethod(Qux $x): Bar<Foo> {
    return $x->bazMethod($x);
  }
}
