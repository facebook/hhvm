<?hh

class Base {
  const type T = this;
  const type U = (function (this): this);
  const type V = shape('foo'=>this, 'bar'=>Map<string,this>);
}

class Child extends Base {
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(Base::class, 'T'));
var_dump(type_structure(Child::class, 'T'));

var_dump(type_structure(Base::class, 'U'));
var_dump(type_structure(Child::class, 'U'));

var_dump(type_structure(Base::class, 'U'));
var_dump(type_structure(Child::class, 'V'));
}
