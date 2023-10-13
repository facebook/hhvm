<?hh

namespace HH\Rx;

interface Traversable<+Tv> extends \HH\Traversable<Tv> {}

interface Iterator<+Tv> extends namespace\Traversable<Tv> {}
