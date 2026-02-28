<?hh

abstract class :mixin {
  children empty;
  attribute int x;
  final public static function foo(): void {}
}

abstract class :action {
  attribute int y;
  final public static function foo(): void {}
}

abstract class :base extends :action {
  attribute :mixin;
}

final class :child extends :base {
  // :base does not inherit foo from :mixin, so there is no conflict between the
  // two final methods. The linearization must provide mro_xhp_attrs_only in
  // order to determine here that :child should not inherit the methods of
  // :mixin.
}
