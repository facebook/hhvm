<?hh
function F()
:mixed{
    $a = "Hello ";
    return($a);
}

abstract final class GStatics {
  public static $myvar = 4;
}

function G()
:mixed{

  echo GStatics::$myvar . " ";
  echo F();
  echo GStatics::$myvar;
}
<<__EntryPoint>> function main(): void {
G();
}
