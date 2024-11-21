<?hh

type T = vec<int>;

<<__EntryPoint>>
function main(): void {
  HH\ReifiedGenerics\get_class_from_type<T>();
}
