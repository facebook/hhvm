<?hh

class A {
    function __get($prop) {
        var_dump($this->$prop);
    }
}
<<__EntryPoint>> function main(): void {
$a = new A();
$prop = null;

var_dump($a->$prop);
}
