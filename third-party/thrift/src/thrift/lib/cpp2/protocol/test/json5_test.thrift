/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/json5"

@thrift.Sealed
struct Nested {
  1: string nested;
}

struct OutOfOrderFields {
  3: i32 Three;
  1: i32 One;
  2: i32 Two;
}

enum Enum {
  NEGATIVE_ONE = -1,
  DEFAULT = 0,
  ONE = 1,
  TWO = 2,
  MANY = 3,
}

struct Example {
  30: optional bool boolValue;
  31: optional byte byteValue;
  32: optional i16 i16Value;
  33: optional i32 i32Value;
  1: optional i64 i64Value;
  2: optional string stringValue;
  3: optional binary binaryValue;
  4: optional float floatValue;
  5: optional double doubleValue;
  6: optional double infValue;
  7: optional double nanValue;
  8: optional list<Nested> listValue;
  // @lint-ignore THRIFTCHECKS bad-key-type
  9: optional set<Nested> setValue;
  10: optional Enum enumValue;
  // @lint-ignore THRIFTCHECKS bad-key-type
  11: optional map<Nested, Nested> structAsKey;
  12: optional map<Enum, Enum> enumAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  13: optional map<bool, i32> boolAsKey;
  14: optional map<i32, i32> i32AsKey;
  15: optional map<binary, i32> binaryAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  16: optional map<list<i32>, i32> listAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  17: optional map<set<i32>, i32> setAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  18: optional map<map<i32, i32>, i32> mapAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  19: optional map<map<map<i32, i32>, i32>, i32> nestedMapAsKey;
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  20: optional set<OutOfOrderFields> outOfOrderFieldsInSet;
  // @lint-ignore THRIFTCHECKS bad-key-type
  21: optional map<OutOfOrderFields, i32> outOfOrderFieldsInMap;
  22: optional map<string, string> stringAsKey;
  23: optional map<i64, i64> i64AsKey;
}

// --
// test expectations are defined here so that they can
// be shared between different languages.
// --

struct TestCase {
  1: string name;
  2: Example example;
  3: string json;
  4: string json5;
}

const list<TestCase> testCases = [
  TestCase{
    name = "Primitive",
    example = Example{
      i64Value = 42,
      stringValue = "hello",
      binaryValue = "?~",
      floatValue = 0.10000000894069671630859375,
      doubleValue = 0.10000000000000001942890293094,
      enumValue = Enum.ONE,
    },
    json = "{
  \"i64Value\": 42,
  \"stringValue\": \"hello\",
  \"binaryValue\": {
    \"utf-8\": \"?~\"
  },
  \"floatValue\": 0.10000001,
  \"doubleValue\": 0.10000000000000002,
  \"enumValue\": \"ONE (1)\"
}",
    json5 = "{
  i64Value: 42,
  stringValue: \"hello\",
  binaryValue: {
    \"utf-8\": \"?~\",
  },
  floatValue: 0.10000001,
  doubleValue: 0.10000000000000002,
  enumValue: \"ONE (1)\",
}",
  },
  TestCase{
    name = "NegativeEnum",
    example = Example{enumValue = Enum.NEGATIVE_ONE},
    json = "{
  \"enumValue\": \"NEGATIVE_ONE (-1)\"
}",
    json5 = "{
  enumValue: \"NEGATIVE_ONE (-1)\",
}",
  },
  TestCase{
    name = "ListValue",
    example = Example{listValue = [Nested{nested = "item1"}]},
    json = "{
  \"listValue\": [
    {
      \"nested\": \"item1\"
    }
  ]
}",
    json5 = "{
  listValue: [
    {
      nested: \"item1\",
    },
  ],
}",
  },
  TestCase{
    name = "MultiElementList",
    example = Example{listValue = [Nested{nested = "a"}, Nested{nested = "b"}]},
    json = "{
  \"listValue\": [
    {
      \"nested\": \"a\"
    },
    {
      \"nested\": \"b\"
    }
  ]
}",
    json5 = "{
  listValue: [
    {
      nested: \"a\",
    },
    {
      nested: \"b\",
    },
  ],
}",
  },
  TestCase{
    name = "SetValue",
    example = Example{setValue = [Nested{nested = "item1"}]},
    json = "{
  \"setValue\": [
    {
      \"nested\": \"item1\"
    }
  ]
}",
    json5 = "{
  setValue: [
    {
      nested: \"item1\",
    },
  ],
}",
  },
  TestCase{
    name = "MultiElementSet",
    example = Example{
      setValue = [Nested{nested = "item2"}, Nested{nested = "item1"}],
    },
    json = "{
  \"setValue\": [
    {
      \"nested\": \"item1\"
    },
    {
      \"nested\": \"item2\"
    }
  ]
}",
    json5 = "{
  setValue: [
    {
      nested: \"item1\",
    },
    {
      nested: \"item2\",
    },
  ],
}",
  },
  TestCase{
    name = "BoolAsKey",
    example = Example{boolAsKey = {1: 1}},
    json = "{
  \"boolAsKey\": [
    {
      \"key\": true,
      \"value\": 1
    }
  ]
}",
    json5 = "{
  boolAsKey: [
    {
      key: true,
      value: 1,
    },
  ],
}",
  },
  TestCase{
    name = "I32AsKey",
    example = Example{i32AsKey = {1: 2}},
    json = "{
  \"i32AsKey\": [
    {
      \"key\": 1,
      \"value\": 2
    }
  ]
}",
    json5 = "{
  i32AsKey: [
    {
      key: 1,
      value: 2,
    },
  ],
}",
  },
  TestCase{
    name = "BinaryAsKey",
    example = Example{binaryAsKey = {"?~": 1}},
    json = "{
  \"binaryAsKey\": [
    {
      \"key\": {
        \"utf-8\": \"?~\"
      },
      \"value\": 1
    }
  ]
}",
    json5 = "{
  binaryAsKey: [
    {
      key: {
        \"utf-8\": \"?~\",
      },
      value: 1,
    },
  ],
}",
  },
  TestCase{
    name = "EnumAsKey",
    example = Example{enumAsKey = {1: 2}},
    json = "{
  \"enumAsKey\": {
    \"ONE (1)\": \"TWO (2)\"
  }
}",
    json5 = "{
  enumAsKey: {
    \"ONE (1)\": \"TWO (2)\",
  },
}",
  },
  TestCase{
    name = "ListAsKey",
    example = Example{listAsKey = {[2, 1]: 3}},
    json = "{
  \"listAsKey\": [
    {
      \"key\": [
        2,
        1
      ],
      \"value\": 3
    }
  ]
}",
    json5 = "{
  listAsKey: [
    {
      key: [
        2,
        1,
      ],
      value: 3,
    },
  ],
}",
  },
  TestCase{
    name = "SetAsKey",
    example = Example{setAsKey = {[2, 1]: 3}},
    json = "{
  \"setAsKey\": [
    {
      \"key\": [
        1,
        2
      ],
      \"value\": 3
    }
  ]
}",
    json5 = "{
  setAsKey: [
    {
      key: [
        1,
        2,
      ],
      value: 3,
    },
  ],
}",
  },
  TestCase{
    name = "MapAsKey",
    example = Example{mapAsKey = {{1: 2}: 3}},
    json = "{
  \"mapAsKey\": [
    {
      \"key\": [
        {
          \"key\": 1,
          \"value\": 2
        }
      ],
      \"value\": 3
    }
  ]
}",
    json5 = "{
  mapAsKey: [
    {
      key: [
        {
          key: 1,
          value: 2,
        },
      ],
      value: 3,
    },
  ],
}",
  },
  TestCase{
    name = "NestedMapAsKey",
    example = Example{nestedMapAsKey = {{{1: 2}: 3}: 4}},
    json = "{
  \"nestedMapAsKey\": [
    {
      \"key\": [
        {
          \"key\": [
            {
              \"key\": 1,
              \"value\": 2
            }
          ],
          \"value\": 3
        }
      ],
      \"value\": 4
    }
  ]
}",
    json5 = "{
  nestedMapAsKey: [
    {
      key: [
        {
          key: [
            {
              key: 1,
              value: 2,
            },
          ],
          value: 3,
        },
      ],
      value: 4,
    },
  ],
}",
  },
  TestCase{
    name = "StructAsKey",
    example = Example{
      structAsKey = {
        Nested{nested = "key2"}: Nested{nested = "value2"},
        Nested{nested = "key1"}: Nested{nested = "value1"},
      },
    },
    json = "{
  \"structAsKey\": [
    {
      \"key\": {
        \"nested\": \"key1\"
      },
      \"value\": {
        \"nested\": \"value1\"
      }
    },
    {
      \"key\": {
        \"nested\": \"key2\"
      },
      \"value\": {
        \"nested\": \"value2\"
      }
    }
  ]
}",
    json5 = "{
  structAsKey: [
    {
      key: {
        nested: \"key1\",
      },
      value: {
        nested: \"value1\",
      },
    },
    {
      key: {
        nested: \"key2\",
      },
      value: {
        nested: \"value2\",
      },
    },
  ],
}",
  },
  TestCase{
    name = "BinaryBase64",
    example = Example{binaryValue = "\xde\xad\xbe\xef"},
    json = "{
  \"binaryValue\": {
    \"base64url\": \"3q2-7w\"
  }
}",
    json5 = "{
  binaryValue: {
    base64url: \"3q2-7w\",
  },
}",
  },
  TestCase{
    name = "BinaryUnprintableUtf8",
    example = Example{binaryValue = "hello\x01"},
    json = "{
  \"binaryValue\": {
    \"base64url\": \"aGVsbG8B\"
  }
}",
    json5 = "{
  binaryValue: {
    base64url: \"aGVsbG8B\",
  },
}",
  },
  TestCase{
    name = "BinaryJsonEscapable",
    example = Example{binaryValue = "a\tb\nc"},
    json = "{
  \"binaryValue\": {
    \"utf-8\": \"a\\tb\\nc\"
  }
}",
    json5 = "{
  binaryValue: {
    \"utf-8\": \"a\\tb\\nc\",
  },
}",
  },
  TestCase{
    name = "OutOfOrderFields",
    example = Example{
      outOfOrderFieldsInSet = [
        OutOfOrderFields{Three = 0, One = 0, Two = 0},
        OutOfOrderFields{Three = 1, One = 1, Two = 1},
        OutOfOrderFields{Three = 0, One = 0, Two = 1},
        OutOfOrderFields{Three = 0, One = 1, Two = 0},
        OutOfOrderFields{Three = 1, One = 0, Two = 0},
        OutOfOrderFields{Three = 1, One = 1, Two = 0},
        OutOfOrderFields{Three = 1, One = 0, Two = 1},
        OutOfOrderFields{Three = 0, One = 1, Two = 1},
        OutOfOrderFields{Three = 0, One = 0, Two = 2},
      ],
    },
    json = "{
  \"outOfOrderFieldsInSet\": [
    {
      \"One\": 0,
      \"Two\": 0,
      \"Three\": 0
    },
    {
      \"One\": 0,
      \"Two\": 0,
      \"Three\": 1
    },
    {
      \"One\": 0,
      \"Two\": 1,
      \"Three\": 0
    },
    {
      \"One\": 0,
      \"Two\": 1,
      \"Three\": 1
    },
    {
      \"One\": 0,
      \"Two\": 2,
      \"Three\": 0
    },
    {
      \"One\": 1,
      \"Two\": 0,
      \"Three\": 0
    },
    {
      \"One\": 1,
      \"Two\": 0,
      \"Three\": 1
    },
    {
      \"One\": 1,
      \"Two\": 1,
      \"Three\": 0
    },
    {
      \"One\": 1,
      \"Two\": 1,
      \"Three\": 1
    }
  ]
}",
    json5 = "{
  outOfOrderFieldsInSet: [
    {
      One: 0,
      Two: 0,
      Three: 0,
    },
    {
      One: 0,
      Two: 0,
      Three: 1,
    },
    {
      One: 0,
      Two: 1,
      Three: 0,
    },
    {
      One: 0,
      Two: 1,
      Three: 1,
    },
    {
      One: 0,
      Two: 2,
      Three: 0,
    },
    {
      One: 1,
      Two: 0,
      Three: 0,
    },
    {
      One: 1,
      Two: 0,
      Three: 1,
    },
    {
      One: 1,
      Two: 1,
      Three: 0,
    },
    {
      One: 1,
      Two: 1,
      Three: 1,
    },
  ],
}",
  },
  TestCase{
    name = "OutOfOrderFieldsInMap",
    example = Example{
      outOfOrderFieldsInMap = {
        OutOfOrderFields{Three = 0, One = 0, Two = 0}: 0,
        OutOfOrderFields{Three = 1, One = 1, Two = 1}: 1,
        OutOfOrderFields{Three = 0, One = 0, Two = 1}: 2,
        OutOfOrderFields{Three = 0, One = 1, Two = 0}: 3,
        OutOfOrderFields{Three = 1, One = 0, Two = 0}: 4,
        OutOfOrderFields{Three = 1, One = 1, Two = 0}: 5,
        OutOfOrderFields{Three = 1, One = 0, Two = 1}: 6,
        OutOfOrderFields{Three = 0, One = 1, Two = 1}: 7,
        OutOfOrderFields{Three = 0, One = 0, Two = 2}: 8,
      },
    },
    json = "{
  \"outOfOrderFieldsInMap\": [
    {
      \"key\": {
        \"One\": 0,
        \"Two\": 0,
        \"Three\": 0
      },
      \"value\": 0
    },
    {
      \"key\": {
        \"One\": 0,
        \"Two\": 0,
        \"Three\": 1
      },
      \"value\": 4
    },
    {
      \"key\": {
        \"One\": 0,
        \"Two\": 1,
        \"Three\": 0
      },
      \"value\": 2
    },
    {
      \"key\": {
        \"One\": 0,
        \"Two\": 1,
        \"Three\": 1
      },
      \"value\": 6
    },
    {
      \"key\": {
        \"One\": 0,
        \"Two\": 2,
        \"Three\": 0
      },
      \"value\": 8
    },
    {
      \"key\": {
        \"One\": 1,
        \"Two\": 0,
        \"Three\": 0
      },
      \"value\": 3
    },
    {
      \"key\": {
        \"One\": 1,
        \"Two\": 0,
        \"Three\": 1
      },
      \"value\": 5
    },
    {
      \"key\": {
        \"One\": 1,
        \"Two\": 1,
        \"Three\": 0
      },
      \"value\": 7
    },
    {
      \"key\": {
        \"One\": 1,
        \"Two\": 1,
        \"Three\": 1
      },
      \"value\": 1
    }
  ]
}",
    json5 = "{
  outOfOrderFieldsInMap: [
    {
      key: {
        One: 0,
        Two: 0,
        Three: 0,
      },
      value: 0,
    },
    {
      key: {
        One: 0,
        Two: 0,
        Three: 1,
      },
      value: 4,
    },
    {
      key: {
        One: 0,
        Two: 1,
        Three: 0,
      },
      value: 2,
    },
    {
      key: {
        One: 0,
        Two: 1,
        Three: 1,
      },
      value: 6,
    },
    {
      key: {
        One: 0,
        Two: 2,
        Three: 0,
      },
      value: 8,
    },
    {
      key: {
        One: 1,
        Two: 0,
        Three: 0,
      },
      value: 3,
    },
    {
      key: {
        One: 1,
        Two: 0,
        Three: 1,
      },
      value: 5,
    },
    {
      key: {
        One: 1,
        Two: 1,
        Three: 0,
      },
      value: 7,
    },
    {
      key: {
        One: 1,
        Two: 1,
        Three: 1,
      },
      value: 1,
    },
  ],
}",
  },
  // ── Small integer types (bool, byte, i16) ─────────────────────────────────
  TestCase{
    name = "SmallIntegers",
    example = Example{boolValue = true, byteValue = 42, i16Value = 1000},
    json = "{
  \"boolValue\": true,
  \"byteValue\": 42,
  \"i16Value\": 1000
}",
    json5 = "{
  boolValue: true,
  byteValue: 42,
  i16Value: 1000,
}",
  },
  // ── Large I64 boundary ──────────────────────────────────────────────────────
  TestCase{
    name = "I64Max",
    example = Example{i64Value = 9223372036854775807},
    json = "{
  \"i64Value\": 9223372036854775807
}",
    json5 = "{
  i64Value: 9223372036854775807,
}",
  },
  // ── Empty struct ────────────────────────────────────────────────────────────
  TestCase{name = "EmptyStruct",example = Example{},json = "{}",json5 = "{}",},
  // ── Empty values ────────────────────────────────────────────────────────────
  TestCase{
    name = "EmptyValues",
    example = Example{stringValue = "", binaryValue = "", listValue = []},
    json = "{
  \"stringValue\": \"\",
  \"binaryValue\": {
    \"utf-8\": \"\"
  },
  \"listValue\": []
}",
    json5 = "{
  stringValue: \"\",
  binaryValue: {
    \"utf-8\": \"\",
  },
  listValue: [],
}",
  },
  // ── String as map key ───────────────────────────────────────────────────────
  TestCase{
    name = "StringAsKey",
    example = Example{stringAsKey = {"hello": "world"}},
    json = "{
  \"stringAsKey\": {
    \"hello\": \"world\"
  }
}",
    json5 = "{
  stringAsKey: {
    hello: \"world\",
  },
}",
  },
  TestCase{
    name = "MultiEntryStringAsKey",
    example = Example{stringAsKey = {"b": "2", "a": "1"}},
    json = "{
  \"stringAsKey\": {
    \"a\": \"1\",
    \"b\": \"2\"
  }
}",
    json5 = "{
  stringAsKey: {
    a: \"1\",
    b: \"2\",
  },
}",
  },
  // ── I64 as map key ──────────────────────────────────────────────────────────
  TestCase{
    name = "I64AsKey",
    example = Example{i64AsKey = {42: 1}},
    json = "{
  \"i64AsKey\": [
    {
      \"key\": 42,
      \"value\": 1
    }
  ]
}",
    json5 = "{
  i64AsKey: [
    {
      key: 42,
      value: 1,
    },
  ],
}",
  },
  // ── Multiple entries in non-enum maps ───────────────────────────────────────
  TestCase{
    name = "MultiEntryI32AsKey",
    example = Example{i32AsKey = {3: 4, 1: 2}},
    json = "{
  \"i32AsKey\": [
    {
      \"key\": 1,
      \"value\": 2
    },
    {
      \"key\": 3,
      \"value\": 4
    }
  ]
}",
    json5 = "{
  i32AsKey: [
    {
      key: 1,
      value: 2,
    },
    {
      key: 3,
      value: 4,
    },
  ],
}",
  },
];
