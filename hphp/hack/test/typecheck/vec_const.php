<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const vec<string> empty_vec = vec[];
const vec<int> vec_const_element = vec[1, 2, 3];
const vec<vec<vec<int>>> nested_vec = vec[vec[vec[1]]];
const vec<(function(): int)> vec_with_illegal_elements = vec[() ==> 1];
