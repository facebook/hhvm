<?hh
class foo {
  <<__DynamicallyCallable>> function cb($param) :mixed{
    var_dump($param);
    return "yes!";
  }
}
<<__EntryPoint>> function main(): void {
error_reporting(0);
$count = -1;
var_dump(preg_replace('', vec[], ''));
var_dump(preg_replace_callback("/(ab)(cd)(e)/", vec[new foo(), "cb"], 'abcde', -1, inout $count));
}
