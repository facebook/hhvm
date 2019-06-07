<?hh

class A {
    function NotAConstructor ()
    {
        if (isset($this->x)) {
            //just for demo
        }
    }
}
<<__EntryPoint>> function main() {
$t = new A ();

print_r($t);
}
