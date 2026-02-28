<?hh

class C<reify T> {}

function f<reify T>(T $a) :mixed{
  return $a;
}
<<__EntryPoint>> function main(): void {
f<C<int>>(new C<int>);
f<C<int>>(new C<string>);
}
