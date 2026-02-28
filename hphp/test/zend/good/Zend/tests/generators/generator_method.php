<?hh

class Test implements IteratorAggregate {
    protected $data;

    public function __construct(varray $data) {
        $this->data = $data;
    }

    public function getIterator() :AsyncGenerator<mixed,mixed,void>{
        foreach ($this->data as $value) {
            yield $value;
        }
    }
}
<<__EntryPoint>> function main(): void {
$test = new Test(vec['foo', 'bar', 'baz']);
foreach ($test as $value) {
    var_dump($value);
}
}
