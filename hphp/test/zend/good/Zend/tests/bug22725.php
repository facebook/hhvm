<?hh
class Foo {
    private function aPrivateMethod() :mixed{
        echo "Foo::aPrivateMethod() called.\n";
    }

    protected function aProtectedMethod() :mixed{
        echo "Foo::aProtectedMethod() called.\n";
        $this->aPrivateMethod();
    }
}

class Bar extends Foo {
    public function aPublicMethod() :mixed{
        echo "Bar::aPublicMethod() called.\n";
        $this->aProtectedMethod();
    }
}
<<__EntryPoint>> function main(): void {
$o = new Bar;
$o->aPublicMethod();
}
