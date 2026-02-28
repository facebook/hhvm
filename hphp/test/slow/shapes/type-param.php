<?hh

type MyOptions = shape(
  'optionA' => bool,
);

class XYZ {
  public function getWidget(
    MyOptions $options = shape('optionA' => true),
  ): MyOptions { return $options; }
}


<<__EntryPoint>>
function main_type_param() :mixed{
var_dump((new XYZ)->getWidget());
}
