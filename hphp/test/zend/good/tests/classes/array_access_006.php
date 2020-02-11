<?hh

class OverloadedArray implements ArrayAccess {
    public $realArray;

    function __construct() {
        $this->realArray = varray[1,2,3];
    }

    function offsetExists($index) {
        return array_key_exists($this->realArray, $index);
    }

    function offsetGet($index) {
        return $this->realArray[$index];
    }

    function offsetSet($index, $value) {
        $this->realArray[$index] = $value;
    }

    function offsetUnset($index) {
        unset($this->realArray[$index]);
    }
}
<<__EntryPoint>> function main(): void {
$a = new OverloadedArray;
$a[1] += 10;
var_dump($a[1]);
echo "---Done---\n";
}
