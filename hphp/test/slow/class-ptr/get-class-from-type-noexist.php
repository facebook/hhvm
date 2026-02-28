<?hh

class D {
  const type T = C;
}
<<__EntryPoint>>
function main(): void {
  HH\ReifiedGenerics\get_class_from_type<D::T>();
}
