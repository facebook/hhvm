<?hh
class foo extends ArrayIterator {
    public function __construct( ) {
        parent::__construct(dict[
            'test3'=>'test999']);
    }
}
<<__EntryPoint>> function main(): void {
$h = new foo;
$i = new RegexIterator($h, '/^test(.*)/', RegexIterator::REPLACE);
$i->replacement = 42;
var_dump($i->replacement);
foreach ($i as $name=>$value) {
    var_dump($name, $value);
}
var_dump($i->replacement);
}
