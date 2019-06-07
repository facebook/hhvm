<?hh

class X {
  function __call($asd, ...$ry) {
    echo "ok\n";
    var_dump($ry);
  }
}



<<__EntryPoint>>
function main_magic_call() {
(new X)->__call(new stdclass, new stdclass, new stdclass);
}
