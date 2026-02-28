<?hh

echo "declaring f()\n";
function f() :mixed{ echo "first def!\n"; }
echo "re-declaring f()\n";
function f() :mixed{ echo "second def!\n"; }
f();

