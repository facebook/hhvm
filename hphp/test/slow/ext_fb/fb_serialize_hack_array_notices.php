<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main(): void {
  // default
  fb_serialize(vec[]);
  fb_serialize(dict[]);
  try {
    fb_serialize(vec[]);
    echo "Serialized: vec\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(dict[]);
    echo "Serialized: dict\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(keyset[]);
    echo "Serialized: keyset\n";
  } catch (Exception $e) {}

  // default
  fb_serialize(vec[], FB_SERIALIZE_VARRAY_DARRAY);
  fb_serialize(dict[], FB_SERIALIZE_VARRAY_DARRAY);
  try {
    fb_serialize(vec[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "Serialized: vec (dvarrays)\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(dict[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "Serialized: dict (dvarrays)\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(keyset[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "Serialized: keyset (dvarrays)\n";
  } catch (Exception $e) {}

  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS);
  try {
    fb_serialize(keyset[], FB_SERIALIZE_HACK_ARRAYS);
    echo "Serialized: keyset (Hack arrays)\n";
  } catch (Exception $e) {}

  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(keyset[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
}
