<?hh

class MyElement extends SimpleXMLElement
{

  public function checkChildClass()
  {
    foreach ($this->children() as $child) {
      if (!$child is MyElement) {
        printf("Hello ");
        break;
      }
    }
    printf("World!\n");
  }

}


// This works fine if we use simplexml_load_string.
<<__EntryPoint>>
function main_child_classes() {
$element = new MyElement('<root><some_child /></root>');
$element->checkChildClass();
}
