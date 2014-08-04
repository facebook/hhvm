<?php
/**
 * FilterIterator which uses a callback for each element
 */
class CallbackFilterIterator extends FilterIterator {

  protected $callback;

  /**
   * Creates a filtered iterator using the callback to determine which
   * items are accepted or rejected.
   *
   * @iterator \Iterator The iterator to be filtered
   * @callback callable The callback, which should return TRUE to accept the
   * current item or FALSE otherwise.
   */
  public function __construct(\Iterator $iterator, callable $callback) {
    parent::__construct($iterator);
    $this->callback = $callback;
  }

  /**
   * Calls the callback with the current value, the current key and the inner
   * iterator as arguments
   *
   * @return bool The callback is expected to return TRUE if the current item
   * is to be accepted, or FALSE otherwise.
   */
  public function accept() {
    return call_user_func(
        $this->callback,
        $this->current(),
        $this->key(),
        $this->getInnerIterator()
    );
  }

}
