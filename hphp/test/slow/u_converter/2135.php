<?hh

class MyConverter extends UConverter {
  public function toUCallback($reason, $source, $codeUnits, inout $error) {
    echo "toUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::toUCallback($reason, $source, $codeUnits, inout $error);
  }
  public function fromUCallback($reason, $source, $codePoint, inout $error) {
    echo "fromUCallback(", UConverter::reasonText($reason), ", ...)\n";
    return parent::fromUCallback($reason, $source, $codePoint, inout $error);
  }
}

function main() {
  using ($c = new MyConverter('ascii', 'utf-8')) {
    $words = varray[
      "regular",
      "irregul\xC1\xA1r",
      "\xC2\xA1unsupported!",
    ];
    foreach($words as $word) {
      $c->convert($word);
    }
  }
}


<<__EntryPoint>>
function main_2135() {
main();
}
