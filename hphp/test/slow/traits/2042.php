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

class a {
 }
interface b {
 }
trait c {
 }
abstract class d {
 }
final class e {
 }
<<__EntryPoint>> function main(): void {
var_dump(get_declared_user_traits());
}
