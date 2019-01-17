<?hh

function f<reify Ta, Tb, <<__Soft>> reify Tc, reify Td>() {}

f<reify int, int, reify string, reify int>(); // no warning
f<reify int, int, string, reify int>(); // warning
f<reify int, int, string, int>(); // warning and then error
