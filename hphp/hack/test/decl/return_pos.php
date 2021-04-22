<?hh

abstract final class DeclPos<T> {
  public static function f1()[]: <<__Soft>> varray_or_darray<T> {return darray[];}
  public static function f2(): <<__Soft>> vec_or_dict<mixed> {return varray[];}
  public static function f3(): <<__Soft>> darray<int, int> {return darray[];}
  public static async function genf4(): <<__Soft>> Awaitable<string> {return "a";}
  public static function f5(): <<__Soft>> bool {return true;}
  public static function f6()[rx_shallow]: <<__Soft>> string {return "a";}
}
