<?hh

class MyConverter extends UConverter {
  public function toUCallback($reason, $source, $codeUnits, inout $error) :mixed{
    $error = U_ZERO_ERROR;
    switch ($codeUnits) {
      case "\x80": return NULL;
      case "\x81": return 'a';
      case "\x82": return ord('b');
      case "\x83": return vec['c'];
      default: break;
    }
  }
  public function fromUCallback($reason, $source, $codePoint, inout $error) :mixed{
    $error = U_ZERO_ERROR;
    switch ($codePoint) {
      case 0x00F1: return "A";
      case 0x00F2: return ord("B");
      case 0x00F3: return vec["C"];
      case 0x00F4: return NULL;
      default: break;
    }
  }
}

<<__EntryPoint>>
function main_2136() :mixed{
$c = new MyConverter('ascii', 'utf-8');
var_dump($c->convert("\x80\x81\x82\x83"));
var_dump($c->convert("\xC3\xB1\xC3\xB2\xC3\xB3\xC3\xB4"));
}
