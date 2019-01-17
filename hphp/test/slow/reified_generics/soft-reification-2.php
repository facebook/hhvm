<?hh

class C<reify Ta, Tb, <<__Soft>> reify Tc, reify Td> {}

new C<reify int, int, reify string, reify int>(); // no warning
new C<reify int, int, string, reify int>(); // warning
new C<reify int, int, string, int>(); // warning and then error
