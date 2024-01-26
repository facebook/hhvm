<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Legacy_FIXME;
use namespace HH\Lib\{C, Math, Dict};

/**
 * Does the PHP style behaviour when doing an inc/dec operation.
 * Specially handles
 *   1. incrementing null
 *   2. inc/dec on empty and numeric strings
 */
function increment(mixed $value)[]: dynamic {
  if ($value is null) {
    return 1;
  }
  if ($value is string) {
    if (\is_numeric($value)) {
      return \HH\str_to_numeric($value) as nonnull + 1;
    }
    if ($value === '') {
      return '1';
    }
  }
  $value as dynamic;
  ++$value;
  return $value;
}

/**
 * See docs on increment
 */
function decrement(mixed $value)[]: dynamic {
  if ($value is string) {
    if (\is_numeric($value)) {
      return \HH\str_to_numeric($value) as nonnull - 1;
    }
    if ($value === '') {
      return -1;
    }
  }

  $value as dynamic;
  --$value;
  return $value;
}

/**
 * Does the PHP style behaviour for casting when doing a mathematical operation.
 * That happens under the following situations
 *   1. null converts to 0
 *   2. bool converts to 0/1
 *   3. numeric string converts to an int or double based on how the string looks.
 *   4. non-numeric string gets converted to 0
 *   5. resources get casted to int
 */
function cast_for_arithmetic(mixed $value)[]: dynamic {
  if ($value is null) {
    return 0;
  }
  if ($value is bool || $value is resource) {
    return (int)$value;
  }
  return $value is string ? \HH\str_to_numeric($value) ?? 0 : $value;
}

/**
 * Does the PHP style behaviour for casting when doing an exponentiation.
 * That happens under the following situations
 *   1. function pointers, and arrays get converted to 0
 *   2. see castForArithmatic
 */
function cast_for_exponent(mixed $value)[]: dynamic {
  if (\HH\is_class_meth($value)) {
    return $value;
  }
  if (\HH\is_fun($value) || $value is AnyArray<_, _>) {
    return 0;
  }
  return cast_for_arithmetic($value);
}

/**
 * Does the PHP style behaviour when doing <, <=, >, >=, <=>.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function lt(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if ($l is string && $r is string) {
    return $l < $r;
  } else if ($l is num && $r is num) {
    return $l < $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::LT) === -1;
}
/**
 * Does the PHP style behaviour when doing <, <=, >, >=, <=>.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function lte(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if ($l is string && $r is string) {
    return $l <= $r;
  } else if ($l is num && $r is num) {
    return $l <= $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::LT) !== 1;
}
/**
 * Does the PHP style behaviour when doing <, <=, >, >=, <=>.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function gt(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if ($l is string && $r is string) {
    return $l > $r;
  } else if ($l is num && $r is num) {
    return $l > $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::GT) === 1;
}
/**
 * Does the PHP style behaviour when doing <, <=, >, >=, <=>.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function gte(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if ($l is string && $r is string) {
    return $l >= $r;
  } else if ($l is num && $r is num) {
    return $l >= $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::GT) !== -1;
}
/**
 * Does the PHP style behaviour when doing <, <=, >, >=, <=>.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function cmp(mixed $l, mixed $r)[]: int {
  // avoid doing slow checks in super common case
  if ($l is string && $r is string) {
    return $l <=> $r;
  } else if ($l is num && $r is num) {
    return $l <=> $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::CMP);
}

/**
 * Does the PHP style behaviour when doing == or ===.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function eq(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if (
    ($l is int && $r is int) ||
    ($l is float && $r is float) ||
    ($l is bool && $r is bool) ||
    ($l is string && $r is string && (!\is_numeric($l) || !\is_numeric($r)))
  ) {
    return $l == $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::EQ) === 0;
}

/**
 * Does the PHP style behaviour when doing == or ===.
 * tl/dr this involves a lot of potential implicit coercions. see
 * __cast_and_compare for the complete picture.
 */
function neq(mixed $l, mixed $r)[]: bool {
  // avoid doing slow checks in super common case
  if (
    ($l is int && $r is int) ||
    ($l is float && $r is float) ||
    ($l is bool && $r is bool) ||
    ($l is string && $r is string && (!\is_numeric($l) || !\is_numeric($r)))
  ) {
    return $l != $r;
  }
  return __cast_and_compare($l, $r, COMPARISON_TYPE::EQ) !== 0;
}

enum COMPARISON_TYPE: int {
  GT = 0;
  LT = 1;
  CMP = 2;
  EQ = 3;
}

/**
 * Do the casts PHP would do before doing <, <=, >, >=, <=>.
 * Then, do a modified form of <=> that correctl handles what gt/lt would do
 * in the different situations
 *
 * Note that this specifically doesn't handle coercions that would just trigger
 * exceptions from within hhvm by default. Instead we trigger the
 * exceptions here manually as part of the <=> invocation
 */
function __cast_and_compare(mixed $l, mixed $r, COMPARISON_TYPE $ctype)[]: int {
  if ($r is bool && !(\HH\is_fun($l) || \HH\is_class_meth($l))) {
    if (!($l is AnyArray<_, _>)) {
      $l = (bool)$l;
    } else if ($ctype === COMPARISON_TYPE::EQ) {
      $l = !C\is_empty($l);
    }
  } else if ($r is null) {
    if ($l is string) {
      $r = '';
    } else if (\is_object($l)) {
      $l = true;
      $r = false;
    } else {
      return __cast_and_compare($l, false, $ctype);
    }
  } else if (
    \HH\is_fun($r) ||
    \HH\is_fun($l) ||
    \HH\is_class_meth($r) ||
    \HH\is_class_meth($l)
  ) {
    // no-op.
  } else if ($r is num) {
    if ($l is null) {
      $l = false;
      $r = (bool)$r;
    } else if ($l is bool) {
      $r = (bool)$r;
    } else if ($l is string) {
      $l = \HH\str_to_numeric($l) ?? 0;
    } else if (
      $l is resource || (\is_object($l) && !($l is \ConstCollection<_>))
    ) {
      $l = $r is int ? (int)$l : (float)$l;
    }
    // if we're ==/!= an int and a float, convert both to float
    if (
      $ctype === COMPARISON_TYPE::EQ &&
      $r is num &&
      $l is num &&
      $r is int !== $l is int
    ) {
      $l = (float)$l;
      $r = (float)$r;
    }
  } else if ($r is string) {
    if ($l is null) {
      $l = '';
    } else if ($l is bool) {
      $r = (bool)$r;
    } else if ($l is num) {
      $r = \HH\str_to_numeric($r) ?? 0;
      return __cast_and_compare($l, $r, $ctype);
    } else if (\is_object($l)) {
      if ($l is \StringishObject && !($l is \ConstCollection<_>)) {
        $l = (string)$l;
      } else if (!($l is \ConstCollection<_>)) {
        $l = true;
        $r = false;
      }
    } else if ($l is resource) {
      $l = (float)$l;
      $r = (float)$r;
    } else if (
      $l is string &&
      $ctype === COMPARISON_TYPE::EQ &&
      \is_numeric($l) &&
      \is_numeric($r)
    ) {
      $l = \HH\str_to_numeric($l);
      $r = \HH\str_to_numeric($r);
      return __cast_and_compare($l, $r, $ctype);
    }
  } else if ($r is resource) {
    if ($l is null) {
      $l = false;
      $r = true;
    } else if ($l is bool) {
      // @lint-ignore CAST_NON_PRIMITIVE 2fax
      $r = (bool)$r;
    } else if ($l is num) {
      // @lint-ignore CAST_NON_PRIMITIVE 2fax
      $r = $l is int ? (int)$r : (float)$r;
    } else if ($l is string) {
      $l = (float)$l;
      // @lint-ignore CAST_NON_PRIMITIVE 2fax
      $r = (float)$r;
    } else if (\is_object($l)) {
      $l = true;
      $r = false;
    }
  } else if ($r is AnyArray<_, _> && ($l is null || $l is bool)) {
    if ($l is null) {
      $l = false;
    }
    if ($ctype === COMPARISON_TYPE::EQ) {
      $r = !C\is_empty($r);
    }
  } else if (
    ($r is vec<_> && $l is vec<_>) ||
    (
      $ctype === COMPARISON_TYPE::EQ &&
      $r is \ConstVector<_> &&
      $l is \ConstVector<_>
    )
  ) {
    if (C\count($l) !== C\count($r)) {
      return C\count($l) > C\count($r) ? 1 : -1;
    }
    foreach ($l as $i => $li) {
      $ri = $r[$i];
      $res = __cast_and_compare($li, $ri, $ctype);
      if ($res !== 0) {
        if (
          $ctype === COMPARISON_TYPE::GT &&
          \is_object($ri) &&
          \is_object($li) &&
          (
            \get_class($li as nonnull) !== \get_class($ri as nonnull) ||
            $li is \Closure
          ) &&
          !($li is \DateTimeInterface && $ri is \DateTimeInterface)
        ) {
          // flip the result :p
          return $res === -1 ? 1 : -1;
        }
        return $res;
      }
    }
    return 0;
  } else if (
    $ctype === COMPARISON_TYPE::EQ &&
    (
      ($r is dict<_, _> && $l is dict<_, _>) ||
      ($r is \ConstMap<_, _> && $l is \ConstMap<_, _>) ||
      ($r is \ConstSet<_> && $l is \ConstSet<_>)
    )
  ) {
    if (C\count($l) !== C\count($r)) return 1;
    foreach ($l as $i => $li) {
      if (
        /* HH_FIXME[4324] I've just confirmed this is safe */
        /* HH_FIXME[4005] Set is KeyedContainer... */
        !C\contains_key($r, $i) || __cast_and_compare($li, $r[$i], $ctype) !== 0
      ) {
        return 1;
      }
    }
    return 0;
  } else if (\is_object($r)) {
    if (
      $l is string && $r is \StringishObject && !($r is \ConstCollection<_>)
    ) {
      $r = (string)$r;
    } else if (
      $l is null ||
      $l is resource ||
      ($l is string && !($r is \ConstCollection<_>))
    ) {
      $l = false;
      $r = true;
    } else if ($l is num && !($r is \ConstCollection<_>)) {
      // this probably throws, but sometimes it doesn't!
      $r = $l is int ? (int)$r : (float)$r;
    } else if ($l is bool) {
      $r = (bool)$r;
    } else if (
      \is_object($l) &&
      !($l is \ConstCollection<_> || $r is \ConstCollection<_>) &&
      $l !== $r &&
      !($l is \DateTimeInterface && $r is \DateTimeInterface)
    ) {
      if (\get_class($l) !== \get_class($r) || $l is \Closure) {
        return $ctype === COMPARISON_TYPE::GT ? -1 : 1;
      } else if (!($l is \SimpleXMLElement)) {
        $l = \HH\object_prop_array($l);
        $r = \HH\object_prop_array($r);
        if (C\count($l) !== C\count($r)) {
          return C\count($l) > C\count($r) ? 1 : -1;
        }

        $loop_var = $ctype === COMPARISON_TYPE::GT ? $r : $l;
        $other_var = $ctype === COMPARISON_TYPE::GT ? $l : $r;
        foreach ($loop_var as $i => $_) {
          if (!C\contains_key($other_var, $i)) {
            // dyn prop in a but not b
            return $ctype === COMPARISON_TYPE::GT ? -1 : 1;
          }
          $li = $l[$i];
          $ri = $r[$i];
          $res = __cast_and_compare($li, $ri, $ctype);
          if ($res !== 0) {
            if (
              ($li is float && Math\is_nan($li)) ||
              ($ri is float && Math\is_nan($ri))
            ) {
              // in the case of NAN && GT, straight up flip the result
              return $ctype === COMPARISON_TYPE::GT && $res === -1 ? 1 : -1;
            }
            return $res;
          }
        }
        return 0;
      }
    }
  }

  if (($l is float && Math\is_nan($l)) || ($r is float && Math\is_nan($r))) {
    // trigger exception if necessary
    $_r = $ctype === COMPARISON_TYPE::EQ
      ? ($l as dynamic) == ($r as dynamic)
      : ($l as dynamic) <=> ($r as dynamic);
    return $ctype === COMPARISON_TYPE::LT ? 1 : -1;
  }

  if ($ctype === COMPARISON_TYPE::EQ) {
    return (int)($l != $r);
  }
  return ($l as dynamic) <=> ($r as dynamic);
}

const int SWITCH_INT_SENTINEL = 7906240793;

/**
 * Do a modification on the switched value. This is in the case where the
 * switched expr is an ?arraykey
 */
function optional_arraykey_to_int_cast_for_switch(?arraykey $value)[]: int {
  if ($value is null) return 0;
  if ($value is string) $value = \HH\str_to_numeric($value) ?? 0;
  if ($value is int) return $value;
  return Math\floor($value) === $value ? (int)$value : SWITCH_INT_SENTINEL;
}

/**
 * Do a modification on the switched value. This is in the case where the
 * switched expr is an ?num
 */
function optional_num_to_int_cast_for_switch(?num $value)[]: int {
  if ($value is int) return $value;
  if ($value is null) return 0;
  return Math\floor($value) === $value ? (int)$value : SWITCH_INT_SENTINEL;

}

/**
   * The rules for coercion when doing a comparison where the RHS is an int
   * are complicated and it's not sufficient to just do straight casts on $value.
   * Instead, we need to do some data tracking to convert the input to specifics
   * values to match specific cases under different circumstances.
   *
   * arraykey instead of int second arg courtesy of non-transparent enums
   */
function int_cast_for_switch(
  mixed $value,
  ?arraykey $first_truthy = null,
)[]: int {
  if ($value is int) return $value;
  if ($value is null) return 0;

  if ($value is string) {
    $value = \HH\str_to_numeric($value) ?? 0;
    if ($value is int) return $value;
    // fallthrough
  }
  if ($value is float) {
    return Math\floor($value) === $value ? (int)$value : SWITCH_INT_SENTINEL;
  }

  if ($value === false) return 0;
  if ($value === true) return $first_truthy as ?int ?? SWITCH_INT_SENTINEL;

  if ($value is resource) return (int)$value;

  if (\is_object($value) && !($value is \ConstCollection<_>)) {
    return (int)($value as dynamic); // this will probably throw
  }

  return SWITCH_INT_SENTINEL;
}


const string SWITCH_STRING_SENTINEL =
  'This string is to force fail matching a case';

/**
 * Do a modification on the switched value. This is in the case where none
 * of the case options are falsy, intish, or floatish
 *
 * arraykey instead of string for second arg courtesy of non-transparent enums
 */
function string_cast_for_basic_switch(
  mixed $value,
  ?arraykey $first_case,
)[]: string {
  if ($value is string) return $value;
  if ($value is null) return '';
  // check for 0ish or true
  if (($value is num && !$value) || ($value is bool && $value)) {
    return ($first_case as ?string) ?? SWITCH_STRING_SENTINEL;
  }
  if ($value is \StringishObject && !($value is \ConstCollection<_>)) {
    return (string)($value as dynamic); // this will throw
  }
  return SWITCH_STRING_SENTINEL;
}

/**
 * The rules for coercion when doing a comparison where the RHS is a string
 * are complicated and it's not sufficient to just do straight casts on $value.
 * Instead, we need to do some data tracking to convert the input to specifics
 * values to match specific cases under different circumstances.
 *
 * arraykey instead of string for second arg courtesy of non-transparent enums *
 */
function string_cast_for_switch(
  mixed $value,
  ?arraykey $first_truthy = null,
  ?arraykey $first_zeroish = null,
  ?arraykey $first_falsy = null,
  dict<arraykey, int> $intish_vals = dict[],
  dict<arraykey, float> $floatish_vals = dict[],
)[]: string {
  $default = SWITCH_STRING_SENTINEL;
  $orig_is_str = $value is string;
  if ($value is string) {
    if (!\is_numeric($value)) return $value;
    $default = $value;
    $value = \HH\str_to_numeric($value) as nonnull;
    // fallthrough
  }

  if ($value is null) return '';

  if ($value is resource) $value = (float)($value);

  if ($value is float) {
    if (Math\floor($value) === $value) {
      $value = (int)$value;
      // fallthrough
    } else {
      if ($orig_is_str) {
        $floatish_vals = Dict\filter_keys($floatish_vals, \is_numeric<>);
      }
      return C\find_key($floatish_vals, $n ==> $n === $value) as ?string ??
        $default;
    }
  }
  if ($value is int) {
    if ($orig_is_str) {
      $intish_vals = Dict\filter_keys($intish_vals, \is_numeric<>);
      // fallthrough
    } else if ($value === 0) {
      return $first_zeroish as ?string ?? $default;
    }
    return C\find_key($intish_vals, $n ==> $n === $value) as ?string ??
      $default;
  }

  if ($value === true) {
    return $first_truthy as ?string ?? SWITCH_STRING_SENTINEL;
  }
  if ($value === false) {
    return $first_falsy as ?string ?? SWITCH_STRING_SENTINEL;
  }

  if ($value is \StringishObject && !($value is \ConstCollection<_>)) {
    return (string)($value as dynamic); // this will throw
  }

  return SWITCH_STRING_SENTINEL;
}
