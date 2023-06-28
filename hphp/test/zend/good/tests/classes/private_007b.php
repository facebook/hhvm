<?hh

class Bar {
    public function pub() :mixed{
        $this->priv();
    }
    private function priv()    :mixed{
        echo "Bar::priv()\n";
    }
}
class Foo extends Bar {
    public function priv()    :mixed{
        echo "Foo::priv()\n";
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Foo();
$obj->pub();
$obj->priv();

echo "Done\n";
}
