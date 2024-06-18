<?hh

module c;

class CPublic {
  public static function in_str_implicit(): string {
    return CInternal::class;
  }

  public static function in_str(): classname<CInternal> {
    return nameof CInternal;
  }

  public static function in_lazy(): classname<CInternal> {
    return CInternal::class;
  }

  public static function in_ptr(): classname<CInternal> {
    return HH\classname_to_class(CInternal::class);
  }
}

internal class CInternal {}
