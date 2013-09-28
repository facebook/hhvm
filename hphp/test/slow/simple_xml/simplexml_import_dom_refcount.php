<?php

function main() {
  $dom = new domDocument;
  $string = <<<END
<a>
  <b>c</b>
</a>
END;

  $dom->loadXML($string);
  $s = simplexml_import_dom($dom);
  $dom = null;
  var_dump((string) $s->b);
}

main();
