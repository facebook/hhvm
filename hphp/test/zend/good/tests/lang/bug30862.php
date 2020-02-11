<?hh

class T {
    static $a = darray[false=>"false", true=>"true"];
}

const X = 0;
const Y = 1;
class T2 {
    static $a = darray[X=>"false", Y=>"true"];
}

<<__EntryPoint>> function main(): void {
print_r(T::$a);

echo "\n----------\n";

print_r(T2::$a);
}
