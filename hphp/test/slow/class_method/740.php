<?php

if (true) {
  class c extends AppendIterator {
}
}
 else {
  class c {
}
}
class d extends c {
  public function rewind() {
    var_dump('rewinding');
  }
}
$obj = new d;
foreach ($obj as $k => $v) {
}
