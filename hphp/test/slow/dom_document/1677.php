<?hh

function rerender($html, $frag = false) :mixed{
    $doc = new DOMDocument();
    if ($frag) {
      $body = $doc->createDocumentFragment();
      $body->appendXML($html);
    }
 else {
      $doc->loadHTML($html);
      $body = $doc->documentElement;
    }
    return helper($body);
  }
  function helper($element) :mixed{
    if ($element is DOMText) {
      return htmlspecialchars($element->nodeValue);
    }
 else {
      $body = '';
      foreach ($element->childNodes as $child) {
        $body .= helper($child);
      }
      if ($element is DOMElement) {
        $attrs = vec[];
        foreach ($element->attributes as $attr) {
          $attrs[] = htmlspecialchars($attr->name) . '="' .             htmlspecialchars($attr->value) . '"';
        }
        if ($attrs) {
          $attrs = ' ' . implode(' ', $attrs);
        }
 else {
          $attrs = '';
        }
        return '<' . $element->tagName . $attrs . '>' . $body .           '</' . $element->tagName . '>';
      }
 else {
        return $body;
      }
    }
  }

  <<__EntryPoint>>
function main_1677() :mixed{
$fragment = 'Hello, <b>world</b>.';
  $document = '<html><body><div style="color:red">    <p class="thing">'.$fragment.'</p></div>';
  echo rerender($fragment, true)."

";
  echo rerender($document, false)."

";
}
