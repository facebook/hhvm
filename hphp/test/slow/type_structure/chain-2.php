<?hh

type T1<T> = T;
type T2 = (T1<int>, T1<string>);

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure('T2'));
}
