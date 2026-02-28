<?hh
/**
 * ( excerpt from
 * http://php.net/manual/en/class.weakref.php )
 *
 * The WeakRef class provides a gateway to objects without preventing the
 * garbage collector from freeing those objects. It also provides a way to turn
 * a weak reference into a strong one.
 */
<<__NativeData>>
final class WeakRef<T> {
  /**
   * ( excerpt from
   * http://php.net/manual/en/weakref.construct.php )
   *
   * Constructs a new weak reference.
   */
  <<__Native>>
  public function __construct(?T $reference)[];

  /**
   * ( excerpt from
   * http://php.net/manual/en/weakref.acquire.php )
   *
   * Acquires a strong reference on that object, virtually turning the weak
   * reference into a strong one.
   */
  <<__Native>>
  public function acquire()[write_props]: bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/weakref.get.php )
   *
   * Returns the object pointed to by the weak reference.
   */
  <<__Native>>
  public function get()[]: ?T;

  /**
   * ( excerpt from
   * http://php.net/manual/en/weakref.release.php )
   *
   * Releases a previously acquired reference, potentially turning a strong
   * reference back into a weak reference.
   */
  <<__Native>>
  public function release()[write_props]: bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/weakref.valid.php )
   *
   * Checks whether the object referenced still exists.
   */
  <<__Native>>
  public function valid()[]: bool;
}
