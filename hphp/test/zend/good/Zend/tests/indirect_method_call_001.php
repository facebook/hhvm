<?hh

class foo {
    public function __construct() {
        throw new Exception('foobar');
    }
}
<<__EntryPoint>> function main(): void {
try {
    $X = (new foo)->Inexistent(3);
} catch (Exception $e) {
    var_dump($e->getMessage()); // foobar
}
}
