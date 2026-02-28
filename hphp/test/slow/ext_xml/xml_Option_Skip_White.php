<?hh
function read_xml($skip_white) :mixed{
  $xml=file_get_contents(__DIR__."/skipwhite.xml");
  $parser=xml_parser_create();
  xml_parser_set_option($parser,XML_OPTION_CASE_FOLDING,0);
  xml_parser_set_option($parser,XML_OPTION_SKIP_WHITE,$skip_white);
  xml_parser_set_option($parser,XML_OPTION_TARGET_ENCODING,"UTF-8");
  $array=vec[];
  $index=vec[];
  xml_parse_into_struct($parser,$xml,inout $array, inout $index);
  return $array;
}

function find_node($array,$node) :mixed{
  foreach($array as $key=>$val) {
    if($val["tag"]==$node) return $val;
  }
  return vec[];
}


// WITH XML_OPTION_SKIP_WHITE=0 WORKS FINE
<<__EntryPoint>>
function main_xml_option_skip_white() :mixed{
$array=read_xml(0);
$node=find_node($array,"query");
print_r($node);

// WITH XML_OPTION_SKIP_WHITE=1 FAILS
$array=read_xml(1);
$node=find_node($array,"query");
print_r($node);
}
