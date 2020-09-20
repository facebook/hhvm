<?hh

class C<reify Ta, reify Tb> {}
class D<reify T> extends C<T> {}
<<__EntryPoint>> function main(): void {
new D<int>();
}
