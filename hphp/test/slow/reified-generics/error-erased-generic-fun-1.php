<?hh

function f<reify T1, T2, reify T3>() :mixed{
}

function g<T>() :mixed{
  f<int, string, T>();
}
<<__EntryPoint>> function main(): void {
g();
}
