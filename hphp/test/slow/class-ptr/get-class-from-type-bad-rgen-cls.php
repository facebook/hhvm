<?hh

class G<reify T> {}

class D {
  const type T = G<int>;
}

<<__EntryPoint>>
function main(): void {
  $g = HH\ReifiedGenerics\get_class_from_type<D::T>();
  new $g(); // match type_structure(D::class, 'T')['classname'] failure
}
