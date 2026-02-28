<?hh
class C {
  private $priv = "ok";

  function doLoop() :mixed{
    echo $this->priv,"\n";
    foreach ($this as $k=>$v) {
      echo "$k: $v\n";
    }
  }
}

class D extends C {
}
<<__EntryPoint>> function main(): void {
$myD = new D;
$myD->doLoop();
}
