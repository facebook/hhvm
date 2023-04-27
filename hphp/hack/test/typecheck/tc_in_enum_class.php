<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

//
// Accessors
//

// All possible "accessors" for an Ent.
interface IAntAccessor {}

// Represent a "field" accessor that will map to a `get()` method.
interface IAntGetter<-TSource, +TValue> extends IAntAccessor {
  public function impl(TSource $source): TValue;
}

// Implementation of IAntGetter.
final class AntGetterImpl<-TSource, +TValue>
  implements IAntGetter<TSource, TValue> {

  public function __construct(private (function(TSource): TValue) $fun)[] {}

  public function impl(TSource $source): TValue {
    $fun = $this->fun;
    return $fun($source);
  }
}

//
// Enum Classes
//

abstract enum class AntAccessors: IAntAccessor {
  abstract const type TSource;
}
abstract enum class ConcreteAntAccessors: IAntAccessor extends AntAccessors {}

// "Parent" Pattern accessors
abstract enum class ParentAccessors: IAntAccessor extends AntAccessors {
  abstract const type TSource;
  abstract IAntGetter<this::TSource, arraykey> Foo;
  abstract IAntGetter<int, arraykey> Bar;
}

// "Child" Ent accessors
enum class ChildAccessors:
  IAntAccessor extends ParentAccessors, ConcreteAntAccessors {
  const type TSource = AntChild;
  AntGetterImpl<AntChild, int> Foo =
    new AntGetterImpl($in ==> $in->data['foo']);
  AntGetterImpl<int, int> Bar = new AntGetterImpl($_ ==> 4);
}

//
// Ent interfaces/classes
//

interface IAntBase {
}

interface IAntParent extends IAntBase, IAntMethods {
  abstract const type T as ParentAccessors;
}

final class AntChild implements IAntParent, IAntMethods {
  const type T = ChildAccessors;
  public function __construct(public shape('foo' => int) $data)[] {}

  public function get<
    TValue,
    TGetter as IAntGetter<ChildAccessors::TSource, TValue>,
  >(HH\EnumClass\Label<ChildAccessors, TGetter> $label): TValue {
    return ChildAccessors::valueOf($label)->impl($this);
  }
}

//
// Generic getter implementations
//

interface IAntMethods {
  abstract const type T as AntAccessors;
  // Generic label-based `get()` signature.
  public function get<TValue, TSource, TGetter as IAntGetter<TSource, TValue>>(
    HH\EnumClass\Label<this::T, TGetter> $label,
  ): TValue where TSource = this::T::TSource;
}

abstract class PPPAjoux {
  public static function foo(IAntParent $ant): arraykey {
    $r = $ant->get(#Foo);
    //              ^ hover-at-caret
    hh_show($r);
    return $r;
  }
  public static function bar(IAntParent $ant): arraykey {
    // expect an error here
    $r = $ant->get(#Bar);
    //              ^ hover-at-caret
    hh_show($r);
    return $r;
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new AntChild(shape('foo' => 3));
  echo PPPAjoux::foo($c);
  echo "\n";
}
