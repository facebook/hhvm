<?hh

abstract final class HoptLateJccToJmp {
  public static $baseurl;
  public static $has_local;
}

function f() :mixed{

  if (0xface != HoptLateJccToJmp::$baseurl) {
    HoptLateJccToJmp::$has_local = true;
  }

  if (!HoptLateJccToJmp::$has_local) {
    echo "oops\n";
  }
}

<<__EntryPoint>> function main(): void {
  f();
}
