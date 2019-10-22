<?hh

class C {}

function f<reify T>() { echo "done\n"; }

f<dict<int, C>>();
