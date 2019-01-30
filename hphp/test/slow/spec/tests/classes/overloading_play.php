<?php

error_reporting(-1);

class PropertyTest
{
    /**  Location for overloaded data.  */
    private $data = array();

    /**  Overloading not used on declared properties.  */
    public $declared = 1;

    /**  Overloading only used on this when accessed outside the class.  */
    private $hidden = 2;

    public function __set($name, $value)
    {
        echo "Setting '$name' to '$value'\n";
        $this->data[$name] = $value;
    }

    public function __get($name)
    {
        echo "Getting '$name'\n";
        if (array_key_exists($name, $this->data)) {
            return $this->data[$name];
        }

        $trace = debug_backtrace();
        trigger_error(
            'Undefined property via __get(): ' . $name .
            ' in ' . $trace[0]['file'] .
            ' on line ' . $trace[0]['line'],
            E_USER_NOTICE);
        return null;
    }

    /**  As of PHP 5.1.0  */
    public function __isset($name)
    {
        echo "Is '$name' set?\n";
        return isset($this->data[$name]);
    }

    /**  As of PHP 5.1.0  */
    public function __unset($name)
    {
        echo "Unsetting '$name'\n";
        unset($this->data[$name]);
    }

    /**  Not a magic method, just here for example.  */
    public function getHidden()
    {
        return $this->hidden;
    }
}

$obj = new PropertyTest;

echo "----------------------\n";

$v = $obj->declared;    // get visible property
echo "declared: $v\n";
$obj->declared = 987;   // set visible property
$v = $obj->declared;    // get visible property
echo "declared: $v\n";

echo "----------------------\n";

$v = $obj->hidden;      // try to get invisible property; can't
                        // Runtime sees that hidden exists, but is invisible,
                        // so calls __get to search for dynamic property, which fails.
var_dump($v);

echo "----------------------\n";

var_dump(isset($obj->hidden));  // test if hidden exists and is accesible, or is dynamic

echo "----------------------\n";

$obj->hidden = "Hello"; // set dynamic invisible property
$v = $obj->hidden;      // get dynamic invisible property
echo "hidden: $v\n";

echo "----------------------\n";

var_dump(isset($obj->hidden));  // test if hidden exists and is accesible, or is dynamic

echo "----------------------\n";

unset($obj->hidden);
var_dump(isset($obj->hidden));
