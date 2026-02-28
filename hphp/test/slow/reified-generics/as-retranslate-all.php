<?hh

class C<T> {}

<<__EntryPoint>>
function main(): void {
  $c = new C();
  $c as C<int>; // T159300246 exception says `Expected C` instead of `Expected C<int>` under retranslate all
}
