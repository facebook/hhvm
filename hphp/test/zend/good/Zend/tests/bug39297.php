<?hh
function compareByRef(inout $first, $id, inout $second) {
    return $first[$id] === $second;
}

class MyTree implements ArrayAccess {
    public $parent;
    public $children = array();

    public function offsetExists($offset) {
    }

    public function offsetUnset($offset) {
    }

    public function offsetSet($offset, $value) {
        echo "offsetSet()\n";
        $cannonicalName = strtolower($offset);
        $this->children[$cannonicalName] = $value;
        $value->parent = $this;
    }

    public function offsetGet($offset) {
        echo "offsetGet()\n";
        $cannonicalName = strtolower($offset);
        return $this->children[$cannonicalName];
    }

}
<<__EntryPoint>> function main(): void {
$id = 'Test';

$root = new MyTree();
$child = new MyTree();
$root[$id] = $child;

var_dump(compareByRef(inout $root,$id, inout $child));
}
