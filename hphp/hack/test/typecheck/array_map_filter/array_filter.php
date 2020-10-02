<?hh // partial

function test_array_filter(
  varray<?string> $vector_array,
  darray<int, ?string> $hashtable_array,
  Container<?string> $container,
  KeyedContainer<int, ?string> $keyed_container,
  int $int,
  $untyped,
): void {
  $f = $x ==> true;
  $intersection_type = $int ? $container : $keyed_container;

  take_string_array(array_filter($vector_array));
  take_int_string_array(array_filter($hashtable_array));
  hh_show(array_filter($container));
  take_int_string_array(array_filter($keyed_container));
  hh_show(array_filter($untyped));
  take_arraykey_string_array(array_filter($intersection_type));

  hh_show(array_filter($vector_array, $f));
  take_int_nullable_string_nullabarray(array_filter($hashtable_array, $f));
  take_arraykey_nullable_string_array(array_filter($container, $f));
  take_int_nullable_string_nullabarray(array_filter($keyed_container, $f));
  hh_show(array_filter($untyped, $f));
  take_arraykey_nullable_string_array(array_filter($intersection_type, $f));
}

function take_string_array(varray<string> $_):void {}
function take_int_string_array(darray<int, string> $_) {}
function take_arraykey_string_array(darray<arraykey, string> $_) {}

function take_int_nullable_string_nullabarray(darray<int, ?string> $_) {}
function take_arraykey_nullable_string_array(darray<arraykey, ?string> $_) {}
