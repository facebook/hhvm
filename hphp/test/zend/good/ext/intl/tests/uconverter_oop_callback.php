<?php
class MyConverter extends UConverter {
  /**
   * Called during conversion from source encoding to internal UChar representation
   */
  public function toUCallback($reason, $source, $codeUnits, &$error) {
    echo "toUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::toUCallback($reason, $source, $codeUnits, $error);
  }

  /**
   * Called during conversion from internal UChar to destination encoding
   */
  public function fromUCallback($reason, $source, $codePoint, &$error) {
    echo "fromUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::fromUCallback($reason, $source, $codePoint, $error);
  }

}

$c = new MyConverter('ascii', 'utf-8');
foreach(array("regular", "irregul\xC1\xA1r", "\xC2\xA1unsupported!") as $word) {
  $c->convert($word);
}