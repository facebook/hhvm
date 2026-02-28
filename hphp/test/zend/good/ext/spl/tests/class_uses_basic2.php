<?hh


trait foo { }
class fooUser { use foo; }

trait bar { }
class barUser { use bar; }

class foobarUser { use foo, bar; }

/** There is no semantics for traits in the inheritance chain.
    Traits are flattend into a class, and that semantics is nothing
    like a type, or interface, and thus, not propergated. */
class fooViaBarUser extends barUser { use foo; }

class fooExtended extends fooUser {}


function s_var_dump($arr) :mixed{
   krsort(inout $arr);
   var_dump($arr);
}
<<__EntryPoint>>
function main_entry(): void {
  /* Prototype  : array class_uses(mixed what [, bool autoload ])
   * Description: Return all traits used by a class
   * Source code: ext/spl/php_spl.c
   * Alias to functions: 
   */

  echo "*** Testing class_uses() : basic ***\n";

  s_var_dump(class_uses(new foobarUser));
  s_var_dump(class_uses('foobarUser'));
  s_var_dump(class_uses(new fooViaBarUser));
  s_var_dump(class_uses('fooViaBarUser'));
  s_var_dump(class_uses(new fooExtended));
  s_var_dump(class_uses('fooExtended'));
  echo "===DONE===\n";
}
