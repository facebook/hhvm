<?hh

class Test implements IteratorAggregate {
    protected $data;

    public function __construct(array $data) {
        $this->data = $data;
    }

    public function getIterator() {
        foreach ($this->data as $value) {
            yield $value;
        }
    }
}
<<__EntryPoint>> function main(): void {
$test = new Test(varray['foo', 'bar', 'baz']);
foreach ($test as $value) {
    var_dump($value);
}
}
