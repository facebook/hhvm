<?hh

function expect_mixed(mixed $_): void {}

interface IChunqPredicate<-T> {
}

final class ChunqFieldInSetPredicate<Tv>
  extends ChunqBinaryPredicate<?Tv, Container<Tv>>
  implements IChunqPredicate<?Tv> {
  final public function getSet(): Container<Tv> {
    throw new Exception("A");
  }
}

final class AlacornInSetPredicate<T as arraykey>
  extends ChunqBinaryPredicate<?T, ConstSet<T>> {
}

abstract class ChunqBinaryPredicate<Tleft, Tright>
  implements IChunqPredicate<Tleft> {
  final public function getValue(): Tright {
    throw new Exception("A");
  }
}

class C {
  public function genApplyPredicate(
    IChunqPredicate<vec<int>> $predicate,
  ): void {
    invariant(
      $predicate is ChunqFieldInSetPredicate<_> ||
        $predicate is AlacornInSetPredicate<_>,
      'Predicate not supported',
    );
    if ($predicate is ChunqFieldInSetPredicate<_>) {
      $privacy_waves = $predicate->getSet();
    } else {
      $privacy_waves = $predicate->getValue();
    }
    expect_mixed($privacy_waves);
  }
}
