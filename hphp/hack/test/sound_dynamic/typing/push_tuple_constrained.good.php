<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

//<<__SupportDynamicType>>
class C {
  public static function updateTime(
    string $time_key,
    IBKSExpression<MinsNum> $new_time,
  ): void {
    $t = tuple(BKSString::const($time_key), $new_time);
    // Works
    BKSMap::const($t);
    // Does not work
    BKSMap::const(tuple(BKSString::const($time_key), $new_time));
  }
}

<<__SupportDynamicType>>
final class BKSMap<+TKey  as supportdyn<mixed> as MinsMapkey, +TValue  as supportdyn<mixed> as BKSClientValue>
  extends BKSExpression<BKSClientMap<TKey, TValue>>
  {

  public static function const(
    (IBKSExpression<TKey>, IBKSExpression<TValue>) $keys_and_values
  ): ~this {
    throw new Exception("A");
  }
}

<<__SupportDynamicType>>
interface MinsMapkey extends MinsNonnull {}
<<__SupportDynamicType>>
interface MinsNonnull extends MinsValue {}
<<__SupportDynamicType>>
interface MinsValue extends MinsNonnullType {
}
<<__SupportDynamicType>>
interface MinsNonnullType {
}
type MinsType = ?MinsNonnullType;

<<__SupportDynamicType>>
interface IBKSExpression<+T  as supportdyn<mixed>>
{
}
type BKSClientValue = ?MinsValue;
type BKSClientMap<+TKey, +TValue> =
  MinsBaseMap<TKey, TValue>;
<<__SupportDynamicType>>
interface MinsBaseMap<+Tk  as supportdyn<mixed>, +Tv  as supportdyn<mixed>>
  extends MinsNonnull {}
<<__SupportDynamicType>>
abstract class BKSExpression<+T  as supportdyn<mixed> as MinsType> implements IBKSExpression<T> { }
<<__SupportDynamicType>>
final class BKSString extends BKSConstant<string, MinsString> {
}
<<__SupportDynamicType>>
abstract class BKSConstant<TBKSServerType as supportdyn<mixed> , +TBKSClientValue  as supportdyn<mixed> as BKSClientValue>
  extends BKSExpression<TBKSClientValue>
 {
  public static function const(TBKSServerType $value): ~this {
    throw new Exception("A");
  }
}
<<__SupportDynamicType>>
interface MinsString extends MinsMapkey, IBKSExpression<MinsString> {
}
<<__SupportDynamicType>>
interface MinsNum extends MinsMapkey {
}
