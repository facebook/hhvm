<?hh

class test {
    function __toString() {
        return "this is test object";
    }
}
<<__EntryPoint>> function main(): void {
$o = new test;
$s = "some string";
$i = 222;
$d = 2323.444;

var_dump($o.$s);
var_dump($o.$i);
var_dump($o.$d);
var_dump($o.$o);

var_dump($s.$o);
var_dump($s.$i);
var_dump($s.$d);
var_dump($s.$s);

var_dump($i.$o);
var_dump($i.$s);
var_dump($i.$d);
var_dump($i.$i);

var_dump($d.$o);
var_dump($d.$s);
var_dump($d.$i);
var_dump($d.$d);

echo "Done\n";
}
