<?hh

class T {
    public static $a = dict[0=>"false", 1=>"true"];
}

const X = 0;
const Y = 1;
class T2 {
    public static $a = dict[X=>"false", Y=>"true"];
}

<<__EntryPoint>> function main(): void {
print_r(T::$a);

echo "\n----------\n";

print_r(T2::$a);
}
