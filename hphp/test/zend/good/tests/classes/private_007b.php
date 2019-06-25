<?hh

class Bar {
    public function pub() {
        $this->priv();
    }
    private function priv()    {
        echo "Bar::priv()\n";
    }
}
class Foo extends Bar {
    public function priv()    {
        echo "Foo::priv()\n";
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Foo();
$obj->pub();
$obj->priv();

echo "Done\n";
}
