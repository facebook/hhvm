<?hh
class foo {
  <<__DynamicallyCallable>> function cb($param) :mixed{
    var_dump($param);
    return "yes!";
  }
}
<<__EntryPoint>> function main(): void {
$count = -1;
var_dump(preg_replace('', varray[], ''));
var_dump(preg_replace_callback("/(ab)(cd)(e)/", varray[new foo(), "cb"], 'abcde', -1, inout $count));
}
