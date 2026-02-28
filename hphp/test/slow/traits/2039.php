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
var_dump($o = serialize(new bar));
var_dump(unserialize($o));
}
