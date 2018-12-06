<?hh

class C<reified +T1, -T2, reified T3>{}

function f<reified T1, T2, reified T3>(int $x) {}

f<reified int, string, reified bool>(1);

new C<reified int, string, reified bool>();
