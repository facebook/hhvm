<?hh

class foo {
    public $value = 42;

    public function &getValue() :mixed{
        return $this->value;
    }
}

