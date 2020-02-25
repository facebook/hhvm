<?hh

class HasCall {
  public function __call($name, $args) {
    echo "__call: name ".print_r($name, true)." args ".
                         print_r($args, true)."\n";
  }
}
<<__EntryPoint>> function main(): void {
$hc = new HasCall();
$hc->CasePreserving(darray[1=>2, 3=>15]);

$methName = "SnooZZZ";
$hc->$methName("llberries");
}
