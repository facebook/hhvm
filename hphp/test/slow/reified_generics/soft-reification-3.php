<?hh

function f<reify Ta, <<__Soft>> reify Tb>() {
  return 1 is Tb;
}
<<__EntryPoint>> function main(): void {
f<int, string>();
f<int, string>();
}
