<?hh

class G<reify T> {}

type T = G<int>;

<<__EntryPoint>>
function main(): void {
  $g = HH\ReifiedGenerics\get_class_from_type<T>();
  new $g(); // match type_structure('T')['classname'] failure
}
