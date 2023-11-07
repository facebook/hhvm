<?hh

<<file: __EnableUnstableFeatures('nameof_class')>>
class A {}
class C extends A {
  const string Cn = nameof C;
  const string Cc = C::class;
  const string Sn = nameof self;
  const string Sc = self::class;
  const string Pn = nameof parent;
  const string Pc = parent::class;
  const string Statn = nameof static;
  const string Statc = static::class;
}
