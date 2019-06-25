<?hh

class State {
    static $a0=10;
    static $counter=0;
    static $a, $b=10, $c=20, $d, $e=30;
    static $s, $k=10;
}

function staticNonStatic() {
    echo "---------\n";
    $a=0;
    echo "$a\n";
    echo State::$a0."\n";
    State::$a0++;
}

function manyInits() {
    echo "------------- Call ".State::$counter." --------------\n";
    echo "Unitialised      : ".State::$a."\n";
    echo "Initialised to 10: ".State::$b."\n";
    echo "Initialised to 20: ".State::$c."\n";
    echo "Unitialised      : ".State::$d."\n";
    echo "Initialised to 30: ".State::$e."\n";
    State::$a++;
    State::$b++;
    State::$c++;
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
   echo State::$s." ".State::$k."\n";
   State::$s++;
   State::$k++;
}
}
