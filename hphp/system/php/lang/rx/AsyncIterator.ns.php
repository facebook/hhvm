<?hh // partial

namespace HH\Rx {

interface AsyncIterator extends \HH\AsyncIterator {
  /**
   * Returns new Awaitable that will produce the next (key, value) Pair, or
   * null if the iteration has finished. It's illegal to call next() while
   * the previously returned Awaitable has not finished yet.
   */
  <<__Rx>>
  public function next()[write_props];
}

}
