<?php

class xml {
  var $parser;
  function xml() {
    $this->parser = xml_parser_create();
    xml_set_object($this->parser, $this);
    xml_set_element_handler($this->parser, 'tag_open', 'tag_close');
    xml_set_character_data_handler($this->parser, 'cdata');
  }
  function parse($data) {
 xml_parse($this->parser, $data);
}
  function tag_open($parser, $tag, $attributes) {
    var_dump($tag, $attributes);
  }
  function cdata($parser, $cdata) {
 var_dump($cdata);
}
  function tag_close($parser, $tag){
 var_dump($tag);
}
}

$xml_parser = new xml();
$xml_parser->parse('<A ID="hallo">PHP</A>');
