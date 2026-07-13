<?hh

trait foo {
    public function serialize() :mixed{
        return 'foobar';
    }
    public function unserialize($x) :mixed{
        var_dump($x);
    }
}

class bar implements Serializable {
    use foo;
}
<<__EntryPoint>> function main(): void {
$o = serialize(new bar); var_dump($o);
var_dump(unserialize($o));
}
