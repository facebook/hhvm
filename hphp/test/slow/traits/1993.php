<?hh

function get_declared_user_traits() :mixed{
  $ret = vec[];
  foreach (get_declared_traits() as $v) {
    // exclude system traits
    $rc = new ReflectionClass($v);
    if ($rc->getFileName() !== false) {
      $ret[] = $v;
    }
  }
  return $ret;
}
class this_is_a_class {
 }
interface this_is_an_interface {
  public function this_is_an_interface_method():mixed;
}
trait this_is_a_trait {
 }
abstract class this_is_an_abstract_class {
 }
final class this_is_a_final_class {
 }
<<__EntryPoint>> function main(): void {
var_dump(get_declared_user_traits());
}
