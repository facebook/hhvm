<?hh
class mycollection implements Iterator {

  private $_elements = varray[];

  public function __construct() {
  }

  public function rewind() {
    $__elements = $this->_elements;
    reset(inout $__elements);
    $this->_elements = $__elements;
  }

  public function count() {
    return count($this->_elements);
  }

  public function current() {
    $__elements = $this->_elements;
    $element = current($__elements);
    $this->_elements = $__elements;
    return $element;
  }

  public function next() {
    $__elements = $this->_elements;
    $element = next(inout $__elements);
    $this->_elements = $__elements;
    return $element;
  }

  public function key() {
    $this->_fillCollection();
    $__elements = $this->_elements;
    $element = key($__elements);
    $this->_elements = $__elements;
    return $element;
  }

  public function valid() {
    throw new Exception('shit happend');

    return ($this->current() !== false);
  }
}

class class2 {
  public $dummy;
}
<<__EntryPoint>> function main(): void {
$obj = new class2();
$col = new mycollection();

try {
    foreach($col as $co) {
      //irrelevant
    }
    echo 'shouldn`t get here';
    //$dummy = 'this will not crash';
    $obj->dummy = 'this will crash';
} catch (Exception $e) {
    echo "ok\n";
}
}
