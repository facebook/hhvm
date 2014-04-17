<?php

class MyConverter extends UConverter {
  public function toUCallback($reason, $source, $codeUnits, &$error) {
    echo "toUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::toUCallback($reason, $source, $codeUnits, $error);
  }
  public function fromUCallback($reason, $source, $codePoint, &$error) {
    echo "fromUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::fromUCallback($reason, $source, $codePoint, $error);
  }
}

function main() {
  $c = new MyConverter('ascii', 'utf-8');
  $words = array(
    "regular",
    "irregul\xC1\xA1r",
    "\xC2\xA1unsupported!",
  );
  foreach($words as $word) {
    $c->convert($word);
  }
  unset($c);
}

main();
