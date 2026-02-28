<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T> {
  public static ?T $fld = null;
  public static vec<T> $vec_fld = vec[];
  private static ?C<T> $private_fld = null;
}

class GenericStorage<Tk as arraykey, TVal> {
  private static Map<Tk, TVal> $storage = Map {};
  public function write(Tk $key, TVal $val): void {
    self::$storage[$key] = $val;
  }
  public function read(Tk $key): TVal {
    return self::$storage[$key];
  }
}
