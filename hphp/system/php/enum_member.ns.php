<?hh // partial

namespace HH {

/* Experimental section for Enum Classes. This is bound to evolve with
 * the HIP process.
 */

/**
 * Base helper class for the enum class feature
 */
final class EnumMember<-TPhantom, +T> {
  public function __construct(private string $name, private T $data) {}

  <<__Pure>>
  public function name(): string {
    return $this->name;
  }

  <<__Pure>>
  public function data(): T {
    return $this->data;
  }
}

}
