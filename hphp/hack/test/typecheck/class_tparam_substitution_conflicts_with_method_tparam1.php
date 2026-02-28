<?hh

abstract class BaseIterable<Tv> {
  public function filterInstanceOf<Tout>(
    classname<Tout> $subclass,
  ): BaseIterable<Tout> where Tout as Tv {
    throw new Exception('unimplemented');
  }
}

abstract class AbstractMappedIterable<Tin, Tout>
  extends BaseIterable<Tout> {}

class MappedIterable<Tv, Tnext>
  extends AbstractMappedIterable<Tv, Tnext> {}
