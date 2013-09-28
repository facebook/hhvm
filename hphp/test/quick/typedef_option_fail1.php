<?hh

type foo = ?int;
function bar(foo $k) {}
bar("fail");

