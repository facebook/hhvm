<?hh

trait my_trait {
  function my_trait_method() :mixed{}
}

class my_base {
  function my_base_method() :mixed{}
  function my_override_method() :mixed{}
}

class my_class extends my_base {
  function my_method() :mixed{}
  use my_trait;
  function my_override_method() :mixed{}
  function my_other_override() :mixed{}
}

class my_child extends my_class {
  function my_other_override() :mixed{}
  function my_child_method() :mixed{}
}

interface I {
  function f(MyClass $c):mixed;
}
<<__EntryPoint>>
function entrypoint_reflection2(): void {

  $rc = new ReflectionClass('my_child');
  foreach ($rc->getMethods() as $ix => $meth) {
    echo $ix, ': ', $meth->getName(), "\n";
    echo 'modifiers: ';   var_dump($meth->getModifiers());
    echo 'isFinal: ';     var_dump($meth->isFinal());
    echo 'isAbstract: ';  var_dump($meth->isAbstract());
    echo 'isPublic: ';    var_dump($meth->isPublic());
    echo 'isProtected: '; var_dump($meth->isProtected());
    echo 'isPrivate: ';   var_dump($meth->isPrivate());
    echo 'isStatic: ';    var_dump($meth->isStatic());
    var_dump($meth->getParameters());

    try {
      $proto = $meth->getPrototype();
      echo 'Prototype: ', $proto->class, '::', $proto->name, "\n";
    } catch (ReflectionException $re) {
      echo get_class($re), ': ', $re->getMessage(), "\n";
    }
  }

  try {
    $r = new ReflectionClass('I');
    foreach ($r->getMethods() as $m) {
      foreach ($m->getParameters() as $p) {
        $p->getClass();
        var_dump($p);
      }
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
