<?hh

class C<reify T> {}

<<__EntryPoint>>
function main(): void {
  var_dump(new C<mixed>() is C<dynamic>);
  var_dump(new C<dynamic>() is C<mixed>);
  var_dump(new C<dynamic>() is C<dynamic>);
}
