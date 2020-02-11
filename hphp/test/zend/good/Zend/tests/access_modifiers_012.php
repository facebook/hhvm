<?hh
class C {
    protected function prot() { }
    private function priv() { }
    public function __call($name, $args)    {
        echo "In __call() for method $name()\n";
    }
}
<<__EntryPoint>> function main(): void {
$c = new C;
call_user_func(varray[$c, 'none']);
call_user_func(varray[$c, 'prot']);
call_user_func(varray[$c, 'priv']);
}
