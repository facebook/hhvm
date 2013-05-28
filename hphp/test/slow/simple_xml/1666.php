<?php

class MyElement extends SimpleXMLElement {
  public function asUcWordString() {
    return ucwords((string)$this);
  }
}
$xml = '<foo><bar><baz>now is the time for all good men to come to the aid of their country</baz></bar></foo>';
$s = simplexml_load_string($xml);
var_dump(get_class($s));
$s = simplexml_load_string($xml, 'MyElement');
var_dump(get_class($s));
var_dump($s->bar->baz->asUcWordString());
