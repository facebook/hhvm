<?php
class C {
  private $priv = "ok";

  function doLoop() {
    echo $this->priv,"\n";
    foreach ($this as $k=>$v) {
      echo "$k: $v\n";
    }
  } 
}

class D extends C {
}

$myD = new D;
$myD->doLoop();
?>