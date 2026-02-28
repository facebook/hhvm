<?hh

function Test ($a) :mixed{
    if ($a<3) {
        return(3);
    }
}

<<__EntryPoint>> function main(): void {
$a = 1;

if ($a < Test($a)) {
    echo "$a\n";
    $a++;
}
}
