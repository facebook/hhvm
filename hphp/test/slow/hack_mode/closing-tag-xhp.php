<?hh

class :script {
  public function __toString()[] :mixed{
    return "Hello World\n";
  }
}


<<__EntryPoint>>
function main_closing_tag_xhp() :mixed{
echo <script></script>;
}
