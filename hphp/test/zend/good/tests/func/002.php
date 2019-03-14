<?php 

abstract final class BlahStatics {
  public static $hey =0;
  public static $yo =0;
}
function blah()
{

  echo "hey=".BlahStatics::$hey++.", ",BlahStatics::$yo--."\n";
}
    
blah();
blah();
blah();
if (isset($hey) || isset($yo)) {
  echo "Local variables became global :(\n";
}
