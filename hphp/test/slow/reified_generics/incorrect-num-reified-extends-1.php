<?hh

class C<reify T> {}
class D<reify T> extends C<T, int> {}
<<__EntryPoint>> function main(): void {
new D<int>();
}
