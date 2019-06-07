<?hh

class A extends NumberFormatter {
    function __construct() {}
}
<<__EntryPoint>> function main() {
$a = new A;
try {
    $b = clone $a;
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
