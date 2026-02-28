<?hh

class ImagickTest extends Imagick {

  /* Protected property */
    protected $test;

  /* Override width property */
  public $width = 112233;

    public function setTestValue($value) :mixed{
        $this->test = $value;
        return $this;
    }

    public function getTestValue() :mixed{
        return $this->test;
    }
}
<<__EntryPoint>> function main(): void {
$test = new ImagickTest("magick:logo");
$test->setTestValue("test value");

echo "Value: " , $test->getTestValue() , PHP_EOL;

var_dump($test->width, $test->height);

echo "OK" , PHP_EOL;
}
