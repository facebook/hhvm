<?hh // partial

namespace HH {

interface AsyncIterator {
  /**
   * Returns new Awaitable that will produce the next (key, value) Pair, or
   * null if the iteration has finished. It's illegal to call next() while
   * the previously returned Awaitable has not finished yet.
   */
  public function next();
}

}
