<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.


abstract class GeneratorDEPRECATEDDataType<Tk, Tv, TkInternal, TvInternal>
  extends CoreDataTypeBase<Tk, Tv, TkInternal, TvInternal> {
}

final class OG
  extends GeneratorDEPRECATEDDataType<string, string, string, string> {
}
abstract class CoreDataTypeBase<Tk, Tv, TkInternal, TvInternal>
  extends GenBasedDataType<Tk, Tv, TkInternal, TvInternal> {
  final public async function genFromStorage(
    TkInternal $id,
  ): Awaitable<TvInternal> { //UNSAFE
  }
}

abstract class GenBasedDataType<Tk, Tv, TkInternal, TvInternal>
  extends AbstractDataType<Tk, Tv>
  implements DataType<TkInternal, TvInternal, this> {
}

abstract class AbstractDataType<Tk, Tv> implements DataTypeImplProvider<this> {}
interface DataType<Tk, +Tv, +Timpl> {}

interface DataTypeImplProvider<+Timpl> {}

async function regenCache(
  string $id,
  DataType<string, string, OG> $data_type,
): Awaitable<bool> {
  invariant($data_type is CoreDataTypeBase<_, _, _, _>, 'hack');
  try {
    await $data_type->genFromStorage($id);
  } catch (Exception $e) {
    return false;
  }
  return true;
}
