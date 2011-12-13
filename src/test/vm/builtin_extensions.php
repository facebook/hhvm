<?php

$classes = array(
"A_Continuation",
"A_GenericContinuation",
"A_DateTime",
"A_DateTimeZone",
"A_DebuggerProxyCmdUser",
"A_DebuggerClient",
"A_DOMNode",
"A_DOMAttr",
"A_DOMCharacterData",
"A_DOMComment",
"A_DOMText",
"A_DOMCDATASection",
"A_DOMDocument",
"A_DOMDocumentFragment",
"A_DOMDocumentType",
"A_DOMElement",
"A_DOMEntity",
"A_DOMEntityReference",
"A_DOMNotation",
"A_DOMProcessingInstruction",
"A_DOMNodeIterator",
"A_DOMNamedNodeMap",
"A_DOMNodeList",
"A_DOMImplementation",
"A_DOMXPath",
"A_EncodingDetector",
"A_EncodingMatch",
"A_SpoofChecker",
"A_ImageSprite",
"A_Collator",
"A_Locale",
"A_Normalizer",
"A_Memcache",
"A_Memcached",
"A_phpmcc",
"A_SimpleXMLElement",
"A_LibXMLError",
"A_SimpleXMLElementIterator",
"A_SoapServer",
"A_SoapClient",
"A_SoapVar",
"A_SoapParam",
"A_SoapHeader",
"A_SQLite3",
"A_SQLite3Stmt",
"A_SQLite3Result",
"A_XMLReader",
"A_XMLWriter",
"A_StringBuffer"
);

class A_Continuation extends Continuation {
  public $___x;
}

class A_GenericContinuation extends GenericContinuation {
  public $___x;
}

class A_DateTime extends DateTime {
  public $___x;
}

class A_DateTimeZone extends DateTimeZone {
  public $___x;
}

class A_DebuggerProxyCmdUser extends DebuggerProxyCmdUser {
  public $___x;
}

class A_DebuggerClient extends DebuggerClient {
  public $___x;
}

class A_DOMNode extends DOMNode {
  public $___x;
}

class A_DOMAttr extends DOMAttr {
  public $___x;
}

class A_DOMCharacterData extends DOMCharacterData {
  public $___x;
}

class A_DOMComment extends DOMComment {
  public $___x;
}

class A_DOMText extends DOMText {
  public $___x;
}

class A_DOMCDATASection extends DOMCDATASection {
  public $___x;
}

class A_DOMDocument extends DOMDocument {
  public $___x;
}

class A_DOMDocumentFragment extends DOMDocumentFragment {
  public $___x;
}

class A_DOMDocumentType extends DOMDocumentType {
  public $___x;
}

class A_DOMElement extends DOMElement {
  public $___x;
}

class A_DOMEntity extends DOMEntity {
  public $___x;
}

class A_DOMEntityReference extends DOMEntityReference {
  public $___x;
}

class A_DOMNotation extends DOMNotation {
  public $___x;
}

class A_DOMProcessingInstruction extends DOMProcessingInstruction {
  public $___x;
}

class A_DOMNodeIterator extends DOMNodeIterator {
  public $___x;
}

class A_DOMNamedNodeMap extends DOMNamedNodeMap {
  public $___x;
}

class A_DOMNodeList extends DOMNodeList {
  public $___x;
}

class A_DOMImplementation extends DOMImplementation {
  public $___x;
}

class A_DOMXPath extends DOMXPath {
  public $___x;
}

class A_EncodingDetector extends EncodingDetector {
  public $___x;
}

class A_EncodingMatch extends EncodingMatch {
  public $___x;
}

class A_SpoofChecker extends SpoofChecker {
  public $___x;
}

class A_ImageSprite extends ImageSprite {
  public $___x;
}

class A_Collator extends Collator {
  public $___x;
}

class A_Locale extends Locale {
  public $___x;
}

class A_Normalizer extends Normalizer {
  public $___x;
}

class A_Memcache extends Memcache {
  public $___x;
}

class A_Memcached extends Memcached {
  public $___x;
}

class A_phpmcc extends phpmcc {
  public $___x;
}

class A_SimpleXMLElement extends SimpleXMLElement {
  public $___x;
}

class A_LibXMLError extends LibXMLError {
  public $___x;
}

class A_SimpleXMLElementIterator extends SimpleXMLElementIterator {
  public $___x;
}

class A_SoapServer extends SoapServer {
  public $___x;
}

class A_SoapClient extends SoapClient {
  public $___x;
}

class A_SoapVar extends SoapVar {
  public $___x;
}

class A_SoapParam extends SoapParam {
  public $___x;
}

class A_SoapHeader extends SoapHeader {
  public $___x;
}

class A_SQLite3 extends SQLite3 {
  public $___x;
}

class A_SQLite3Stmt extends SQLite3Stmt {
  public $___x;
}

class A_SQLite3Result extends SQLite3Result {
  public $___x;
}

class A_XMLReader extends XMLReader {
  public $___x;
}

class A_XMLWriter extends XMLWriter {
  public $___x;
}

class A_StringBuffer extends StringBuffer {
  public $___x;
}

foreach($classes as $cls) {
  echo $cls . "\n";
  $a = new $cls;
  var_dump($a);
  if ($a instanceof Closure || $a instanceof Continuation) {
    continue;
  }
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if ($a != $c) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));
}

// call a sample method in DateTime
$a = new A_DateTime("now");
echo $a->getTimezone()->getName() . "\n";
