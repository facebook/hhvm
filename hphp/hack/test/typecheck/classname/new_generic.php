<?hh

<<__ConsistentConstruct>>
abstract class C<T> {
  public function __construct(private T $data) {}
}

function factory_c<Targ, Tobj as C<Targ>>(
  classname<Tobj> $klass,
  Targ $arg,
): Tobj {
  $inst = new $klass($arg);
  hh_show($inst);
  return $inst;
}

function test_factory(classname<C<int>> $klass) {
  hh_show(factory_c($klass, 10));
  hh_show(factory_c($klass, 'string'));
}
