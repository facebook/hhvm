<?php

class MyElement extends SimpleXMLElement
{

  public function checkChildClass()
  {
    foreach ($this->children() as $child) {
      if (!$child instanceof MyElement) {
        printf("Hello ");
        break;
      }
    }
    printf("World!\n");
  }

}

// This works fine if we use simplexml_load_string.
$element = new MyElement('<root><some_child /></root>');
$element->checkChildClass();
