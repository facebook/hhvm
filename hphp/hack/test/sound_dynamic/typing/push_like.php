<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function get():~int {
  return 3;
}

async function gen():Awaitable<~int> {
  return 3;
}

function return_pair_direct():~(int,string) {
  return tuple(get(), "A");
}

function return_pair():~(int,string) {
  $x = tuple(get(), "A");
  return $x;
}

function return_vec_direct():~vec<int> {
  return vec[get()];
}

function return_vec():~vec<int> {
  $x = vec[get()];
  return $x;
}

function return_dict_direct():~dict<string,int> {
  return dict['a' => get()];
}

function return_dict():~dict<string,int> {
  $x = dict['a' => get()];
  return $x;
}

function return_vec_awaitable_direct():~vec<Awaitable<int>> {
  return vec[gen()];
}

function return_vec_awaitable():~vec<Awaitable<int>> {
  $x = vec[gen()];
  return $x;
}

function return_shape_direct():~shape('a' => int, 'b' => string) {
  return shape('a' => get(), 'b' => "A");
}

function return_shape():~shape('a' => int, 'b' => string) {
  $x = shape('a' => get(), 'b' => "A");
  return $x;
}

function return_dict_vec_direct():~dict<string,vec<int>> {
  return dict['a' => vec[get()]];
}

function return_dict_vec():~dict<string,vec<int>> {
  $x = dict['a' => vec[get()]];
  return $x;
}
