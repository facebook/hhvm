<?php
class BUG {
  public $error = "please fix this thing, it wasted a nice part of my life!\n";
  static function instance() {return new BUG();}  

  function __destruct()
  {
    $c=get_class($this); unset($c);
    echo get_class($this) ."\n";
    if(defined('DEBUG_'.__CLASS__)){}
    $c=get_class($this); //memory leak only
    echo $this->error;
  }
}


BUG::instance()->error;
echo "this is still executed\n";
?>