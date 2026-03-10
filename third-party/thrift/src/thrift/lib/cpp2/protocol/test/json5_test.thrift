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
  20: optional set<OutOfOrderFields> outOfOrderFields;
}

// --
// test expectations are defined here so that they can
// be shared between different languages.
// --

const Example primitiveExample = Example{
  i64Value = 42,
  stringValue = "hello",
  binaryValue = "?~",
  floatValue = 0.10000000894069671630859375,
  doubleValue = 0.10000000000000001942890293094,
  enumValue = Enum.ONE,
};
const string primitiveJson = "{
  \"i64Value\": 42,
  \"stringValue\": \"hello\",
  \"binaryValue\": {
    \"utf-8\": \"?~\"
  },
  \"floatValue\": 0.10000001,
  \"doubleValue\": 0.10000000000000002,
  \"enumValue\": \"ONE (1)\"
}";
const string primitiveJson5 = "{
  i64Value: 42,
  stringValue: \"hello\",
  binaryValue: {
    \"utf-8\": \"?~\",
  },
  floatValue: 0.10000001,
  doubleValue: 0.10000000000000002,
  enumValue: \"ONE (1)\",
}";

const Example listValueExample = Example{
  listValue = [Nested{nested = "item1"}],
};
const string listValueJson = "{
  \"listValue\": [
    {
      \"nested\": \"item1\"
    }
  ]
}";
const string listValueJson5 = "{
  listValue: [
    {
      nested: \"item1\",
    },
  ],
}";

const Example multiElementListExample = Example{
  listValue = [Nested{nested = "a"}, Nested{nested = "b"}],
};
const string multiElementListJson = "{
  \"listValue\": [
    {
      \"nested\": \"a\"
    },
    {
      \"nested\": \"b\"
    }
  ]
}";
const string multiElementListJson5 = "{
  listValue: [
    {
      nested: \"a\",
    },
    {
      nested: \"b\",
    },
  ],
}";

const Example setValueExample = Example{setValue = [Nested{nested = "item1"}]};
const string setValueJson = "{
  \"setValue\": [
    {
      \"nested\": \"item1\"
    }
  ]
}";
const string setValueJson5 = "{
  setValue: [
    {
      nested: \"item1\",
    },
  ],
}";

const Example multiElementSetExample = Example{
  setValue = [Nested{nested = "item2"}, Nested{nested = "item1"}],
};
const string multiElementSetJson = "{
  \"setValue\": [
    {
      \"nested\": \"item1\"
    },
    {
      \"nested\": \"item2\"
    }
  ]
}";
const string multiElementSetJson5 = "{
  setValue: [
    {
      nested: \"item1\",
    },
    {
      nested: \"item2\",
    },
  ],
}";

const Example boolAsKeyExample = Example{boolAsKey = {1: 1}};
const string boolAsKeyJson = "{
  \"boolAsKey\": [
    {
      \"key\": true,
      \"value\": 1
    }
  ]
}";
const string boolAsKeyJson5 = "{
  boolAsKey: [
    {
      key: true,
      value: 1,
    },
  ],
}";

const Example i32AsKeyExample = Example{i32AsKey = {1: 2}};
const string i32AsKeyJson = "{
  \"i32AsKey\": [
    {
      \"key\": 1,
      \"value\": 2
    }
  ]
}";
const string i32AsKeyJson5 = "{
  i32AsKey: [
    {
      key: 1,
      value: 2,
    },
  ],
}";

const Example binaryAsKeyExample = Example{binaryAsKey = {"?~": 1}};
const string binaryAsKeyJson = "{
  \"binaryAsKey\": [
    {
      \"key\": {
        \"utf-8\": \"?~\"
      },
      \"value\": 1
    }
  ]
}";
const string binaryAsKeyJson5 = "{
  binaryAsKey: [
    {
      key: {
        \"utf-8\": \"?~\",
      },
      value: 1,
    },
  ],
}";

const Example enumAsKeyExample = Example{enumAsKey = {1: 2}};
const string enumAsKeyJson = "{
  \"enumAsKey\": {
    \"ONE (1)\": \"TWO (2)\"
  }
}";
const string enumAsKeyJson5 = "{
  enumAsKey: {
    \"ONE (1)\": \"TWO (2)\",
  },
}";

const Example listAsKeyExample = Example{listAsKey = {[2, 1]: 3}};
const string listAsKeyJson = "{
  \"listAsKey\": [
    {
      \"key\": [
        2,
        1
      ],
      \"value\": 3
    }
  ]
}";
const string listAsKeyJson5 = "{
  listAsKey: [
    {
      key: [
        2,
        1,
      ],
      value: 3,
    },
  ],
}";

const Example setAsKeyExample = Example{setAsKey = {[2, 1]: 3}};
const string setAsKeyJson = "{
  \"setAsKey\": [
    {
      \"key\": [
        1,
        2
      ],
      \"value\": 3
    }
  ]
}";
const string setAsKeyJson5 = "{
  setAsKey: [
    {
      key: [
        1,
        2,
      ],
      value: 3,
    },
  ],
}";

const Example mapAsKeyExample = Example{mapAsKey = {{1: 2}: 3}};
const string mapAsKeyJson = "{
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
}";
const string mapAsKeyJson5 = "{
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
}";

const Example nestedMapAsKeyExample = Example{
  nestedMapAsKey = {{{1: 2}: 3}: 4},
};
const string nestedMapAsKeyJson = "{
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
}";
const string nestedMapAsKeyJson5 = "{
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
}";

const Example structAsKeyExample = Example{
  structAsKey = {
    Nested{nested = "key2"}: Nested{nested = "value2"},
    Nested{nested = "key1"}: Nested{nested = "value1"},
  },
};
const string structAsKeyJson = "{
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
}";
const string structAsKeyJson5 = "{
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
}";

const Example binaryBase64Example = Example{binaryValue = "\xde\xad\xbe\xef"};
const string binaryBase64Json = "{
  \"binaryValue\": {
    \"base64url\": \"3q2-7w\"
  }
}";
const string binaryBase64Json5 = "{
  binaryValue: {
    base64url: \"3q2-7w\",
  },
}";

const Example outOfOrderFieldsExample = Example{
  outOfOrderFields = [
    OutOfOrderFields{Three = 0, One = 1, Two = 0},
    OutOfOrderFields{Three = 1, One = 0, Two = 0},
  ],
};
const string outOfOrderFieldsJson = "{
  \"outOfOrderFields\": [
    {
      \"One\": 1,
      \"Two\": 0,
      \"Three\": 0
    },
    {
      \"One\": 0,
      \"Two\": 0,
      \"Three\": 1
    }
  ]
}";
const string outOfOrderFieldsJson5 = "{
  outOfOrderFields: [
    {
      One: 1,
      Two: 0,
      Three: 0,
    },
    {
      One: 0,
      Two: 0,
      Three: 1,
    },
  ],
}";
