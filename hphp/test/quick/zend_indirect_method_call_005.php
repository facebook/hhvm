<?hh

class foo implements arrayaccess {
    private $container = array();
    public function __construct($container) {
      $this->container = $container;
    }
    public function offsetSet($offset, $value) {
        if (is_null($offset)) {
            $this->container[] = $value;
        } else {
            $this->container[$offset] = $value;
        }
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
<<__EntryPoint>> function main(): void {
var_dump( (new foo( array(1, array(4, 5), 3) ))[1][0] ); // int(4)
}
