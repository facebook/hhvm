<?hh

class MyConverter extends UConverter {
  private dict<string, int> $lastReasons = dict[];

  private function logCallback(string $name, int $reason): void {
    $is_repeatable =
      $reason === UConverter::REASON_ILLEGAL ||
      $reason === UConverter::REASON_UNASSIGNED;
    $is_repeated = ($this->lastReasons[$name] ?? null) === $reason;
    $this->lastReasons[$name] = $reason;
    if ($is_repeatable && $is_repeated) return;
    echo $name, "(", UConverter::reasonText($reason), ", ...)\n";
  }

  public function toUCallback($reason, $source, $codeUnits, inout $error) :mixed{
    $this->logCallback('toUCallback', $reason);
    return parent::toUCallback($reason, $source, $codeUnits, inout $error);
  }
  public function fromUCallback($reason, $source, $codePoint, inout $error) :mixed{
    $this->logCallback('fromUCallback', $reason);
    return parent::fromUCallback($reason, $source, $codePoint, inout $error);
  }
}

function main() :mixed{
  using ($c = new MyConverter('ascii', 'utf-8')) {
    $words = vec[
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
function main_2135() :mixed{
main();
}
