<?hh
<<__SupportDynamicType>>
interface I {
}

<<__SupportDynamicType>>
interface J
  <
    Tent as supportdyn<mixed> as I
  > {

  public static function title(
  ): void;
  }

<<__SupportDynamicType>>
trait TR
  <
    Tent as I,
    Tspec as supportdyn<mixed> as J<Tent>
  > {

  abstract const classname<Tspec> SPEC_CLASS;

  public function whereTitle(
  ): void {
    $spec_class = static::SPEC_CLASS;
    $spec_class::title();
  }
}
