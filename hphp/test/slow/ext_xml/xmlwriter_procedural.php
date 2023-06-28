<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "got: $x\n"; }
}

function VERIFY($x) :mixed{
  VS($x, true);
}


<<__EntryPoint>>
function main_xmlwriter_procedural() :mixed{
$xml = xmlwriter_open_memory();
var_dump($xml);
VERIFY(xmlwriter_set_indent($xml, true));
VERIFY(xmlwriter_set_indent_string($xml, "  "));
VERIFY(xmlwriter_start_document($xml, "1.0", "utf-8"));

VERIFY(xmlwriter_start_element($xml, "node"));
VERIFY(xmlwriter_write_attribute($xml, "name", "value"));
VERIFY(xmlwriter_start_attribute($xml, "name2"));
VERIFY(xmlwriter_end_attribute($xml));
VERIFY(xmlwriter_write_element($xml, "subnode", "some text"));
VERIFY(xmlwriter_end_element($xml));

VERIFY(xmlwriter_start_element_ns($xml, "fb", "node",
                                  "http://www.facebook.com/"));
VERIFY(xmlwriter_write_attribute_ns($xml, "fb", "attr",
                                    "http://www.facebook.com/", "value"));
VERIFY(xmlwriter_start_attribute_ns($xml, "fb", "attr2",
                                    "http://www.facebook.com/"));
VERIFY(xmlwriter_end_attribute($xml));
VERIFY(xmlwriter_write_element_ns($xml, "prefix", "name",
                                  "http://some.url/", '1337'));
VERIFY(xmlwriter_start_element($xml, "node"));
VERIFY(xmlwriter_full_end_element($xml));
VERIFY(xmlwriter_end_element($xml));

VERIFY(xmlwriter_start_element($xml, "node"));
VERIFY(xmlwriter_start_cdata($xml));
VERIFY(xmlwriter_text($xml, "Raw text"));
VERIFY(xmlwriter_end_cdata($xml));
VERIFY(xmlwriter_end_element($xml));

VERIFY(xmlwriter_start_element($xml, "node"));
VERIFY(xmlwriter_write_cdata($xml, "More CDATA"));
VERIFY(xmlwriter_end_element($xml));

VERIFY(xmlwriter_start_comment($xml));
VERIFY(xmlwriter_text($xml, "Comments"));
VERIFY(xmlwriter_end_comment($xml));

VERIFY(xmlwriter_write_comment($xml, "More comments"));

VERIFY(xmlwriter_start_pi($xml, "lol"));
VERIFY(xmlwriter_end_pi($xml));
VERIFY(xmlwriter_write_pi($xml, "hh", "print 'Hello world!';"));

VERIFY(xmlwriter_write_raw($xml, "<node>Raw XML</node>"));

VERIFY(xmlwriter_write_dtd($xml, "name", "publicID", "systemID", "subset"));
VERIFY(xmlwriter_start_dtd($xml, "name", "publicID", "systemID"));
VERIFY(xmlwriter_end_dtd($xml));

VERIFY(xmlwriter_start_dtd_element($xml, "name"));
VERIFY(xmlwriter_end_dtd_element($xml));
VERIFY(xmlwriter_write_dtd_element($xml, "name", "content"));

VERIFY(xmlwriter_start_dtd_attlist($xml, "name"));
VERIFY(xmlwriter_end_dtd_attlist($xml));
VERIFY(xmlwriter_write_dtd_attlist($xml, "name", "content"));

VERIFY(xmlwriter_start_dtd_entity($xml, "name", false));
VERIFY(xmlwriter_end_dtd_entity($xml));
VERIFY(xmlwriter_write_dtd_entity($xml, "name", "content", false, "publicid",
                                  "systemid", "ndataid"));

VERIFY(xmlwriter_end_document($xml));

var_dump(xmlwriter_flush($xml));
var_dump(xmlwriter_output_memory($xml));
}
