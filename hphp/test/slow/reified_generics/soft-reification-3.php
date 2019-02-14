<?hh

function f<reify Ta, <<__Soft>> reify Tb>() {
  return 1 is Tb;
}

f<int, string>();
f<int, string>();
