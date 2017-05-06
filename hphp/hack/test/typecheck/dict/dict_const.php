<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

const dict<string, string> empty_dict = dict[];
const dict<int, string>
  dict_const_element = dict[1 => 'foo', 2 => 'bar', 3 => 'baz'];

const dict<string, vec<dict<string, int>>>
  nested_dict = dict['foo' => vec[dict['baz' => 5]]];

const dict<string, (function(): int)>
  dict_with_illegal_elements = dict['foo' => () ==> 1];
