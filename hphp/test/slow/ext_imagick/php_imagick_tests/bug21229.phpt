--TEST--
Test PECL bug #21229
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

class ImagickTest extends Imagick {

  /* Protected property */
    protected $test;

  /* Override width property */
  public $width = 112233;

    public function setTestValue($value) {
        $this->test = $value;
        return $this;
    }

    public function getTestValue() {
        return $this->test;
    }
}

$test = new ImagickTest("magick:logo");
$test->setTestValue("test value");

echo "Value: " , $test->getTestValue() , PHP_EOL;

var_dump($test->width, $test->height);

echo "OK" , PHP_EOL;


?>
--EXPECTF--
Value: test value
int(112233)
int(%d)
OK
