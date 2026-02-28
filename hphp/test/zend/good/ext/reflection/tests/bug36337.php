<?hh

abstract class enum {
    protected $_values;

    public function __construct() {
        $property = new ReflectionProperty(get_class($this),'_values');
        var_dump($property->isProtected());
    }

}

final class myEnum extends enum {
    public $_values = dict[
        0 => 'No value',
    ];
}
<<__EntryPoint>> function main(): void {
$x = new myEnum();

echo "Done\n";
}
