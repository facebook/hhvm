<?hh

class :script {
  public function __toString() {
    return "Hello World\n";
  }
}


<<__EntryPoint>>
function main_closing_tag_xhp() {
echo <script></script>;
}
