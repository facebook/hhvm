<?hh

type MyOptions = shape(
  'optionA' => bool,
);

class XYZ {
  public function getWidget(
    MyOptions $options = shape('optionA' => true),
  ): MyOptions { return $options; }
}

var_dump((new XYZ())->getWidget());
