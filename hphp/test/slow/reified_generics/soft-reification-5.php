<?hh

class D<reify Ta, reify Tb> {}
class C<reify Ta, <<__Soft>> reify Tb> extends D<Ta, Tb> {}
<<__EntryPoint>> function main(): void {
$c = new C<int, string>();
}
