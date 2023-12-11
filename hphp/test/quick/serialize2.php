<?hh

interface I {}
class A { }
class B extends A { }
class C implements I { }
class D extends C { }
class E extends B implements I { }
class UnexpectedSerializedClass extends Exception {}

<<__EntryPoint>> function main(): void {
  error_reporting(E_ALL);
  $v = serialize(vec[new A, new B, new C, new D, new E]);
  $run = $opts ==> {
    try {
      printf("%s (%s)\n",
             join("", array_map(
                    $x ==> get_class($x)[0],
                    unserialize($v, $opts))),
             json_encode($opts));
    } catch (Exception $e) {
      printf("%s %s %s\n", get_class($e), $e->getMessage(), json_encode($opts));
    }
  };
  foreach (vec[false, true] as $subclasses) {
    $check = $xs ==> $run(shape('include_subclasses' => $subclasses,
                                'allowed_classes' => $xs));
    $check(vec[]);
    $check(vec['A']);
    $check(vec['B']);
    $check(vec['C']);
    $check(vec['D']);
    $check(vec['E']);
    $check(vec['I']);
    $check(vec['A', 'I']);
    $check(vec['A', 'I']);
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => vec['A','B','C','D','E','I'],
               'throw' => 'UnexpectedSerializedClass'));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => vec['A','I'],
               'throw' => 'UnexpectedSerializedClass'));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => false,
               'throw' => false));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => false,
               'throw' => 'UnexpectedSerializedClass'));
  }
  unserialize('O:3:"Foo":0:{}',
    shape('allowed_classes' => vec['I']));
  unserialize('O:3:"Bar":0:{}',
    shape('allowed_classes' => vec['Feh'],
          'include_subclasses' => true));
  unserialize('O:3:"Baz":0:{}',
    shape('allowed_classes' => vec['Fiddle'],
          'include_subclasses' => true,
          'throw' => 'MadeUp'));
}
