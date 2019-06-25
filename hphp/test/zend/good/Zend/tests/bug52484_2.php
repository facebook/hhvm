<?hh

class A {
    function __set($prop, $val) {
        $this->$prop = $val;
    }
}
<<__EntryPoint>> function main(): void {
$a = new A();
$prop = null;

$a->$prop = 2;
}
