<?hh

class str {}
class str1 extends str { public function __toString()[] :mixed{ return "a"; } }
class str2 extends str {
  public function __toString()[] :mixed{ throw new Exception('a'); }
}

function bar(str $k) :mixed{
  $tmp = 54;
  try {
    $y = $k->__toString();
    $tmp = 2;
  } catch (Exception $x) {
    var_dump($tmp);
  }
  var_dump($tmp);
}



<<__EntryPoint>>
function main_dce_004() :mixed{
bar(new str1);
bar(new str2);
}
