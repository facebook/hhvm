<?hh

function foo() :mixed{
    $x = vec[1,2,3];
    foreach ($x as $a) {
        while (1) {
            throw new Exception();
        }
        return;
    }
}

<<__EntryPoint>> function main(): void {
try {
    foo();
} catch (Exception $ex) {
    echo "ok\n";
}
}
