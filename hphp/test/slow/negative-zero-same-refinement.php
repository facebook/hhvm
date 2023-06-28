<?hh

function positive($a) :mixed{
    if ($a === 0.) {
        echo "it's positive?\n";
        var_dump($a);
    }
}

function negative($a) :mixed{
    $minuszero = 1 / -\INF;
    if ($a === $minuszero) {
        echo "it's negative?\n";
        var_dump($a);
    }
}

<<__EntryPoint>>
function main() :mixed{
    $a = 1 / -\INF;
    var_dump($a);
    positive($a);
    $b = 0.0;
    negative($b);
}
