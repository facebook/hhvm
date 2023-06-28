<?hh

class test {
    function __toString() :mixed{
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
var_dump((string)($o).(string)($d));
var_dump($o.$o);

var_dump($s.$o);
var_dump($s.$i);
var_dump($s.(string)($d));
var_dump($s.$s);

var_dump($i.$o);
var_dump($i.$s);
var_dump($i.(string)($d));
var_dump($i.$i);

var_dump((string)($d).(string)($o));
var_dump((string)($d).$s);
var_dump((string)($d).$i);
var_dump((string)($d).(string)($d));

echo "Done\n";
}
