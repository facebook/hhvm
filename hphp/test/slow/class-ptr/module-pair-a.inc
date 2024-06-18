<?hh

module a;

class APublic {
  public static function in_str_implicit(): string {
    return AInternal::class;
  }

  public static function in_str(): classname<AInternal> {
    return nameof AInternal;
  }

  public static function in_lazy(): classname<AInternal> {
    return AInternal::class;
  }

  public static function in_ptr(): classname<AInternal> {
    return HH\classname_to_class(AInternal::class);
  }
}

internal class AInternal {}
