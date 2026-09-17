(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Fixed language protocols shared by independent generation primitives. *)

let xhp =
  {|
abstract class MilnerXhpBase {
  public function __construct(
    public darray<string, mixed> $attributes,
    public varray<mixed> $children,
    string $_filename,
    int $_line,
  )[] {}
  public function getAttribute(string $name, mixed $default = null)[]: mixed {
    return $this->attributes[$name] ?? $default;
  }
}
// T288868908: direct structural attribute hints typecheck but fail emission.
final class MilnerPayload<T> {
  public function __construct(public T $value)[] {}
}
|}

let expression_tree =
  {|
type MilnerPos = shape(...);
interface Spliceable<TVisitor, +TResult, +TInfer> {
  public function visit(TVisitor $visitor): TResult;
}
final class MilnerTree<T> implements Spliceable<MilnerDsl, mixed, T> {
  public function __construct(private (function(MilnerDsl): mixed) $ast)[] {}
  public function visit(MilnerDsl $visitor): mixed { return ($this->ast)($visitor); }
}
final class MilnerDsl {
  const type TAst = mixed;
  public static function makeTree<T>(
    ?MilnerPos $_pos,
    shape(
      'splices' => dict<string,mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
      ?'type' => (function(): T),
      'variables' => vec<string>,
      'lexically_enclosing_tree' => ?MilnerPos,
    ) $_metadata,
    (function(MilnerDsl): mixed) $ast,
  )[]: MilnerTree<T> { return new MilnerTree($ast); }
  public static function valueTree<T>(T $value)[]: MilnerTree<T> {
    return new MilnerTree($_ ==> $value);
  }
  public static function lift<T>(MilnerTree<T> $tree)[]: MilnerTree<T> { return $tree; }
  public function splice<T>(?MilnerPos $_pos, string $_key, MilnerTree<T> $tree, ?vec<string> $_vars = null): mixed { return $tree->visit($this); }
}
|}
