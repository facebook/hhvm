<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

interface DataTypeImplProvider<Timpl> {
  public function impl(): Timpl;
}

interface DataType<Tk, Tv, Timpl>
  extends DataTypeImplProvider<Timpl> {

  public function get(Tk $id): Tv;
}

abstract class AbstractDataType<Tk, Tv> implements DataTypeImplProvider<this> {
  final static public function at(): DataType<Tk, Tv, this> {
    invariant_violation('');
  }

  final public function impl(): this {
    return $this;
  }
}

class MyThingDataType extends AbstractDataType<int, string> {
  public function get(int $v): string {
    return (string)$v;
  }

  public function customMethod(): void {
  }
}

function myFunc(): string {
  MyThingDataType::at()->impl()->customMethod();
  return MyThingDataType::at()->get(1);
}
