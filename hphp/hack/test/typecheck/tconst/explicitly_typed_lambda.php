<?hh // strict

interface IMyContainer {
  abstract const type TMyElem;
  public function getElements(
    (function(this::TMyElem): bool) $predicate,
  ): Traversable<this::TMyElem>;
}

abstract class MyViewBase {
  abstract const type TMyContainer as IMyContainer;
  const type TMyElem = this::TMyContainer::TMyElem;

  abstract protected function condition(this::TMyElem $elem): bool;

  public function getElements(
    this::TMyContainer $container,
  ): Traversable<this::TMyElem> {
    return $container->getElements(
      (this::TMyContainer::TMyElem $elem) ==> $this->condition($elem),
    );
  }
}
