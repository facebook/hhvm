<?hh // strict

namespace HH;

function stringish_cast(mixed $value): string {
  if (HH\is_fun($value)) {
    return HH\fun_get_function($value);
  }
  if (is_object($value) && $value is Stringish) {
    /* HH_IGNORE_ERROR[4128] This is intentionally deprecated */
    return $value->__toString();
  }
  return (string)$value;
}
