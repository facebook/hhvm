<?hh

trait MyTrait{
}

class C {
  const type T = MyTrait;
  const type U = C::T::T1;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'U'));
}
