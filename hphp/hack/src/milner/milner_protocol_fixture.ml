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
