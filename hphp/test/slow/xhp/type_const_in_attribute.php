<?hh

abstract class base {
  protected static function __xhpAttributeDeclaration()[]: darray {
    return dict[];
  }
}

abstract class :base:component extends base {
  abstract const type TStyleShape as shape(?'width' => ?int, ...) =
    shape(?'width' => ?int, ...);

  attribute
    this::TStyleShape style_shape;

  public static function dumpAttributes(): void {
    var_dump(static::__xhpAttributeDeclaration());
  }
}

final class :child:example extends :base:component {
  const type TStyleShape = shape(?'width' => ?int, ?'height' => ?int);
}

<<__EntryPoint>>
function main(): void {
  :child:example::dumpAttributes();
  :base:component::dumpAttributes();
}
