<?hh

function f<reify Ta, <<__Soft>> reify Tb>() {
  return 1 is Tb;
}

f<reify int, reify string>();
f<reify int, string>();
