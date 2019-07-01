<?hh
class a {
    public $b;
}
class b {
    public $c;
}
class c {
    public $d;
}
<<__EntryPoint>> function main(): void {
$a = new a();
$a->b = new b();
$a->b->c = new c();
$a->b->c->d = $a;
var_dump(unserialize(serialize($a)));
}
