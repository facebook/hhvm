<?hh

abstract final class XSLTPrepare {

  <<__Memoize>>
  public static function getDOM() {
    $dom = new DOMDocument;
    $dom->load(dirname(__FILE__)."/xslt.xml");
    if(!$dom) {
      echo "Error while parsing the document\n";
      exit;
    }
    return $dom;
  }

  <<__Memoize>>
  public static function getXSL() {
    $xsl = new DOMDocument;
    $xsl->load(dirname(__FILE__)."/xslt.xsl");
    if(!$xsl) {
      echo "Error while parsing the document\n";
      exit;
    }
    return $xsl;
  }

  <<__Memoize>>
  public static function getProc() {
    $proc = new xsltprocessor;
    if(!$proc) {
      echo "Error while making xsltprocessor object\n";
      exit;
    }
    return $proc;
  }
}
