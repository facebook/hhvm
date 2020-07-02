<?hh

class T {
    static $a = darray[0=>"false", 1=>"true"];
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
