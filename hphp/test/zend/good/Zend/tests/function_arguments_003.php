<?hh
const a = 10;

function t1($a = 1 + 1, $b = 1 << 2, $c = "foo" . "bar", $d = a * 10) :mixed{
    var_dump($a, $b, $c, $d);
}
<<__EntryPoint>> function main(): void {
t1();
}
