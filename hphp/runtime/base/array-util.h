/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_ARRAY_UTIL_H_
#define incl_HPHP_ARRAY_UTIL_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/util/hdf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Utility array functions.
 */
class ArrayUtil {
public:
  /////////////////////////////////////////////////////////////////////////////
  // Compositions.

  /**
   * Create an array using the elements of the first parameter as keys
   * each initialized to value.
   */
  static Variant CreateArray(CArrRef keys, CVarRef value);

  /**
   * Create an array containing num elements starting with index start_key
   * each initialized to value.
   */
  static Variant CreateArray(int start_index, int num, CVarRef value);

  /**
   * Split array into chunks.
   */
  static Variant Chunk(CArrRef input, int size, bool preserve_keys = false);

  /**
   * Taking a slice. When "preserve_keys" is true, a vector will turn
   * into numerically keyed map. When "preserve_keys" is false, a map will
   * turn into vectors, unless keys are not numeric.
   */
  static Variant Slice(CArrRef input, int offset, int64_t length,
                       bool preserve_keys);

  /**
   * Removes the elements designated by offset and length and replace them
   * with supplied array.
   */
  static Variant Splice(CArrRef input, int offset, int64_t length = 0,
                        CVarRef replacement = null_variant,
                        Array *removed = nullptr);

  /**
   * Returns a copy of input array padded with pad_value to size pad_size.
   */
  static Variant Pad(CArrRef input, CVarRef pad_value, int pad_size,
                     bool pad_right = true);

  /**
   * Create an array containing the range of integers or characters from low
   * to high (inclusive).
   */
  static Variant Range(unsigned char low, unsigned char high, int64_t step = 1);
  static Variant Range(double low, double high, double step = 1.0);
  static Variant Range(double low, double high, int64_t step = 1);

  /**
   * Conversion between HDF and array.
   */
  static Variant FromHdf(const Hdf &hdf);
  static void ToHdf(const Array &arr, Hdf &hdf);

  /////////////////////////////////////////////////////////////////////////////
  // Information and calculations.

  /**
   * Computes the sum of the array entries as int64 or double.
   */
  static DataType Sum(CArrRef input, int64_t *isum, double *dsum);

  /**
   * Computes the product of the array entries as int64 or double.
   */
  static DataType Product(CArrRef input, int64_t *iprod, double *dprod);

  /**
   * Return the value as key and the frequency of that value in input
   * as value.
   */
  static Variant CountValues(CArrRef input);

  /////////////////////////////////////////////////////////////////////////////
  // Manipulations. Note, all these functions will create a new array than
  // modifying input, although names of these functions sound like mutating.

  /**
   * Retuns an array with all string keys lowercased [or uppercased].
   */
  static Variant ChangeKeyCase(CArrRef input, bool lower);

  /**
   * Return input as a new array with the order of the entries reversed.
   */
  static Variant Reverse(CArrRef input, bool preserve_keys = false);

  /**
   * Randomly shuffle the contents of an array.
   */
  static Variant Shuffle(CArrRef input);

  /**
   * Return key/keys for random entry/entries in the array.
   */
  static Variant RandomKeys(CArrRef input, int num_req = 1);

  /**
   * Removes duplicate string values from array.
   */
  static Variant StringUnique(CArrRef input);

  /**
   * Removes values whose numeric conversion is duplicate from array.
   */
  static Variant NumericUnique(CArrRef input);

  /**
   * Removes values that compare as equal and that end up in contiguous
   * positions if the input array is sorted.
   */
  static Variant RegularSortUnique(CArrRef input);

  /////////////////////////////////////////////////////////////////////////////
  // Iterations.

  /**
   * Apply a user function to every member of an array.
   */
  typedef void (*PFUNC_WALK)(VRefParam value, CVarRef key, CVarRef userdata,
                             const void *data);
  static void Walk(VRefParam input, PFUNC_WALK walk_function, const void *data,
                   bool recursive = false, PointerSet *seen = nullptr,
                   CVarRef userdata = null_variant);

  /**
   * Iteratively reduce the array to a single value via the callback.
   */
  typedef Variant (*PFUNC_REDUCE)(CVarRef result, CVarRef operand,
                                  const void *data);
  static Variant Reduce(CArrRef input, PFUNC_REDUCE reduce_function,
                        const void *data, CVarRef initial = null_variant);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_UTIL_H_
