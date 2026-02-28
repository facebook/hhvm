<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class DictConsts {
  const dict<int, string> CDICT = dict[2 => "folly"];
  const dict<int, dict<int, string>> CNESTED_DICT =
    dict[2 => dict[4 => "folly"]];
  const dict<int, dict<int, float>> CNESTED_DICT_FLOAT =
    dict[2 => dict[4 => -4.5e10, 5 => 5.0]];
}

abstract class AbstractConsts {
  abstract const int CABSTRACT_INT;
}

class VecConsts {
  const vec<int> CVEC = vec[1, 2, 3];
  const vec<vec<int>> CNESTED_VEC = vec[vec[1], vec[2], vec[3]];
}

class ArrayConsts {
  const dict<string, int> CDARRAY = dict["test" => 1];
  const keyset<string> CKEYSET = keyset['a', 'b'];
  const keyset<classname> CCLASSNAME_KEYSET = keyset[];
  const vec<string> CVARRAY = vec['MAP_1', 'MAP_2'];
}

class BinopConsts {
  const int CINT = 4 + 5;
  const float CFLOAT = 4.0 - 5.0;
  const string CSTRING = "test"."test";
  const bool CBOOL = true || false;
}

class ShapeConsts {
  const shape('a' => int, 'b' => string, ...) CSHAPE =
    shape('a' => 42, 'b' => 'foo', 'c' => 3.14);
}

class TupleConsts {
  const (int, ?(string, float)) COPTION = tuple(7, null);
}

class ClassnameConsts {
  const classname<TupleConsts> CCLASSNAME = TupleConsts::class;
  const classname<ClassnameConsts> CCLASSNAME2 = self::class;
}
