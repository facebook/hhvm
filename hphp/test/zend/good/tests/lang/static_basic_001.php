<?hh

class State {
    public static $a0=10;
    public static $counter=0;
    public static $a, $b=10, $c=20, $d, $e=30;
    public static $s, $k=10;
}

function staticNonStatic() :mixed{
    echo "---------\n";
    $a=0;
    echo "$a\n";
    echo State::$a0."\n";
    State::$a0++;
}

function manyInits() :mixed{
    echo "------------- Call ".State::$counter." --------------\n";
    echo "Unitialised      : ".(string)(State::$a)."\n";
    echo "Initialised to 10: ".State::$b."\n";
    echo "Initialised to 20: ".State::$c."\n";
    echo "Unitialised      : ".(string)(State::$d)."\n";
    echo "Initialised to 30: ".State::$e."\n";
    State::$a ??= 0;
    State::$a++;
    State::$b++;
    State::$c++;
    State::$d ??= 0;
    State::$d++;
    State::$e++;
    State::$counter++;
}

<<__EntryPoint>> function main(): void {
echo "\nSame variable used as static and non static.\n";

staticNonStatic();
staticNonStatic();
staticNonStatic();

echo "\nLots of initialisations in the same statement.\n";

manyInits();
manyInits();
manyInits();

echo "\nUsing static keyword at global scope\n";
for ($i=0; $i<3; $i++) {
   echo (string)(State::$s)." ".(string)(State::$k)."\n";
   State::$s ??= 0;
   State::$s++;
   State::$k++;
}
}
