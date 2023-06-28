<?hh

class MyElement extends SimpleXMLElement
{

  public function checkChildClass()
:mixed  {
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
function main_child_classes() :mixed{
$element = new MyElement('<root><some_child /></root>');
$element->checkChildClass();
}
