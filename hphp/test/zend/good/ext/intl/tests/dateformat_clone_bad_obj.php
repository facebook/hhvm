<?hh

class A extends IntlDateFormatter {
    function __construct() {}
}
<<__EntryPoint>> function main(): void {
$a = new A;
try {
    $b = clone $a;
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
