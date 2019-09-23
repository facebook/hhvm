<?hh

class foo {
    public $value = 42;

    public function &getValue() {
        return $this->value;
    }
}

