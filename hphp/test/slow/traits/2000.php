<?hh

trait MY_TRAIT {
 }
class MY_CLASS {
 use MY_TRAIT;
 }
<<__EntryPoint>> function main(): void {
$r = new ReflectionClass('MY_CLASS');
var_dump($r->getTraitNames());
}
