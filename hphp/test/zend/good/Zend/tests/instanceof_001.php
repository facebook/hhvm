<?hh
<<__EntryPoint>> function main(): void {
$a = new stdClass;
var_dump($a is stdClass);

var_dump(new stdClass is stdClass);

$b = () ==> new stdClass;
var_dump($b() is stdClass);

$c = vec[new stdClass];
var_dump($c[0] is stdClass);

try {
    var_dump($inexistent is stdClass);
} catch (Exception $e) {
    var_dump($e->getMessage());
}

var_dump("$a" is stdClass);
}
