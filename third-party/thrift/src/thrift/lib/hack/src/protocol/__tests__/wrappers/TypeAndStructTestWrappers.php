<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class MyTypeWrapper<TThriftType>
  extends \IThriftTypeWrapper<TThriftType> {

  <<__Override>>
  public static async function genToThrift(
    this $value,
  )[zoned_shallow]: Awaitable<TThriftType> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift<<<__Explicit>> TThriftType__>(
    TThriftType__ $value,
  )[zoned_shallow]: Awaitable<MyTypeWrapper<TThriftType__>> {
    return new MyTypeWrapper($value);
  }

  <<__Override>>
  public async function genUnwrap()[zoned_shallow]: Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(
    TThriftType $value,
  )[zoned_shallow]: Awaitable<void> {
    $this->value = $value;
  }

  public static function verifySame<TThriftType__>(
    ?MyTypeWrapper<TThriftType__> $obj1,
    ?MyTypeWrapper<TThriftType__> $obj2,
  ): bool {
    if ($obj1 === $obj2) {
      return true;
    }

    return $obj1?->value === $obj2?->value;
  }
}

final class MyStructWrapper<TThriftType as ?IThriftStruct>
  extends \IThriftStructWrapper<TThriftType> {

  <<__Override>>
  public static async function genToThrift(
    this $value,
  )[zoned_shallow]: Awaitable<TThriftType> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift<
    <<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(
    TThriftType__ $value,
  )[zoned_shallow]: Awaitable<MyStructWrapper<TThriftType__>> {
    return new MyStructWrapper($value);
  }

  <<__Override>>
  public async function genUnwrap()[zoned_shallow]: Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(
    TThriftType $value,
  )[zoned_shallow]: Awaitable<void> {
    $this->value = $value;
  }

  public static function verifySame<TThriftType__ as ?IThriftStruct>(
    ?MyStructWrapper<TThriftType__> $obj1,
    ?MyStructWrapper<TThriftType__> $obj2,
  ): bool {
    if ($obj1 === $obj2) {
      return true;
    }

    expect($obj1)->toBePHPEqual($obj2);
    return true;
  }
}

final class MyTypeIntWrapper<TThriftType>
  extends \IThriftTypeWrapper<TThriftType> {

  <<__Override>>
  public static async function genToThrift(
    this $value,
  )[zoned_shallow]: Awaitable<TThriftType> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift<<<__Explicit>> TThriftType__>(
    TThriftType__ $value,
  )[zoned_shallow]: Awaitable<MyTypeIntWrapper<TThriftType__>> {
    return new MyTypeIntWrapper($value);
  }

  <<__Override>>
  public async function genUnwrap()[zoned_shallow]: Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(
    TThriftType $value,
  )[zoned_shallow]: Awaitable<void> {
    $this->value = $value;
  }

  public static function verifySame<TThriftType__>(
    ?MyTypeIntWrapper<TThriftType__> $obj1,
    ?MyTypeIntWrapper<TThriftType__> $obj2,
  ): bool {
    if ($obj1 === $obj2) {
      return true;
    }

    return $obj1?->value === $obj2?->value;
  }
}

final class MyAdapter1 implements IThriftAdapter {
  const type TThriftType = int;
  const type THackType = string;
  public static function fromThrift(int $thrift_value)[]: string {
    return (string)$thrift_value;
  }

  public static function toThrift(string $hack_value)[]: int {
    return (int)$hack_value;
  }
}
