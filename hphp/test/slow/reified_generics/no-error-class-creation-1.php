<?hh

class C{}

new C<reify int, reify string, reify int>();

echo "done\n";
