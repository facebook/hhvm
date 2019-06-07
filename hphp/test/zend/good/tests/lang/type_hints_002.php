<?hh
class P { }
class T {
    function f(P $p = NULL) {
        var_dump($p);
        echo "-\n";
    }
}
<<__EntryPoint>> function main() {
$o=new T();
$o->f(new P);
$o->f();
$o->f(NULL);
}
