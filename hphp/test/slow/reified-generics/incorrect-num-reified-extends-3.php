<?hh

class C<reify Ta, reify Tb> {}
class D extends C<int> {}
<<__EntryPoint>> function main(): void {
new D();
}
