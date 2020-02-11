<?hh

class obj implements arrayaccess {
    private $container = array();
    public function __construct() {
        $this->container = darray[
            "one"   => 1,
            "two"   => 2,
            "three" => 3,
        ];
    }
    public function offsetSet($offset, $value) {
        $this->container[$offset] = $value;
    }
    public function offsetExists($offset) {
        return isset($this->container[$offset]);
    }
    public function offsetUnset($offset) {
        unset($this->container[$offset]);
    }
    public function offsetGet($offset) {
        return isset($this->container[$offset]) ? $this->container[$offset] : null;
    }
}

function x() {
	return new obj;
}

<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);
  var_dump(x()['two']);
}
