<?hh


// From https://github.com/facebook/hhvm/issues/2337

<<__EntryPoint>>
function main_filter_unsafe_raw_strip() :mixed{
var_dump(
  filter_var(
    "a\ta",
    FILTER_UNSAFE_RAW,
    FILTER_FLAG_STRIP_LOW | FILTER_FLAG_NO_ENCODE_QUOTES
  )
);
}
