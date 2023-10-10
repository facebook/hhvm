<?hh

interface IBase {
  const type T = this;
}

interface IChild extends IBase {
  abstract const type V;
}

class Base implements IBase {
  const type U = this;
}

class Child extends Base implements IChild {
  const type V = this;
}

class GrandChild extends Child {
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(GrandChild::class, 'T'));
var_dump(type_structure(IChild::class, 'T'));
var_dump(type_structure(GrandChild::class, 'U'));
var_dump(type_structure(GrandChild::class, 'V'));
}
