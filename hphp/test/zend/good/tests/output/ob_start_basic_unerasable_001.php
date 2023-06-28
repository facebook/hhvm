<?hh

abstract final class CallbackStatics {
  public static $callback_invocations = 0;
}
function callback($string) :mixed{
    CallbackStatics::$callback_invocations++;
    return "[callback:" . CallbackStatics::$callback_invocations . "]$string\n";
}
<<__EntryPoint>> function main(): void {
ob_start(callback<>, 0, 0);

echo "This call will obtain the content:\n";
$str = ob_get_contents();
var_dump($str);
echo "==DONE==";
}
