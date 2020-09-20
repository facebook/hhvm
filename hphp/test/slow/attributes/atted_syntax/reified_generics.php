<?hh

class C<@__Soft reify T> {}

function f(C<int> $_): void {}

@__EntryPoint
function main(): void {
  f(new C<string>());
}
