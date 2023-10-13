<?hh

type ANY<T> = T;
/* HH_FIXME[4101] trick to create Tany type */
type TANY = ANY;

abstract class Base {
  const type TData = TANY;

  private function __construct(protected this::TData $data) {}

  final protected function getData(): this::TData {
    return $this->data;
  }
}

abstract class Middle extends Base {
  const type TData  = string;
}

trait ChildTrait1 {
  require extends Middle;
}

trait ChildTrait2 {
  require extends Base;
}

final class Child extends Middle {
  // in folded decl, this::TData comes from Base OR Middle, depending on order of used traits below
  // `check_ambiguous_inheritance` aims to force explicit redeclaration in such case,
  // but it's not triggered in presence of Tany (which is both subtype and supertype of string)

  // in shallow decl, we always deterministically choose TData = string
  use ChildTrait1;
  use ChildTrait2;

  public function test(): int {
    return $this->getData();
  }
}
