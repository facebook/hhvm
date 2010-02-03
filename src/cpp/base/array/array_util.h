/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_ARRAY_UTIL_H__
#define __HPHP_ARRAY_UTIL_H__

#include <cpp/base/type_array.h>

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
  static Array CreateArray(CArrRef keys, CVarRef value);

  /**
   * Create an array containing num elements starting with index start_key
   * each initialized to value.
   */
  static Array CreateArray(int start_index, int num, CVarRef value);

  /**
   * Creates an array by using the elements of the first parameter as keys
   * and the elements of the second as the corresponding values.
   */
  static Array Combine(CArrRef keys, CArrRef values);

  /**
   * Split array into chunks.
   */
  static Array Chunk(CArrRef input, int size, bool preserve_keys = false);

  /**
   * Taking a slice. When "preserve_keys" is true, a vector will turn
   * into numerically keyed map. When "preserve_keys" is false, a map will
   * turn into vectors, unless keys are not numeric.
   */
  static Array Slice(CArrRef input, int offset, int length,
                     bool preserve_keys);

  /**
   * Removes the elements designated by offset and length and replace them
   * with supplied array.
   */
  static Array Splice(CArrRef input, int offset, int length = 0,
                      CVarRef replacement = null_variant, Array *removed = NULL);

  /**
   * Returns a copy of input array padded with pad_value to size pad_size.
   */
  static Array Pad(CArrRef input, CVarRef pad_value, int pad_size,
                   bool pad_right = true);

  /**
   * Create an array containing the range of integers or characters from low
   * to high (inclusive).
   */
  static Array Range(unsigned char low, unsigned char high, int step = 1);
  static Array Range(double low, double high, double step = 1.0);
  static Array Range(int64 low, int64 high, int64 step = 1);

  /////////////////////////////////////////////////////////////////////////////
  // Information and calculations.

  /**
   * Returns the sum of the array entries.
   */
  static double Sum(CArrRef input);

  /**
   * Returns the product of the array entries.
   */
  static double Product(CArrRef input);

  /**
   * Return the value as key and the frequency of that value in input
   * as value.
   */
  static Array CountValues(CArrRef input);

  /////////////////////////////////////////////////////////////////////////////
  // Manipulations. Note, all these functions will create a new array than
  // modifying input, although names of these functions sound like mutating.

  /**
   * Retuns an array with all string keys lowercased [or uppercased].
   */
  static Array ChangeKeyCase(CArrRef input, bool lower);

  /**
   * Return array with key <-> value flipped.
   */
  static Array Flip(CArrRef input);

  /**
   * Return input as a new array with the order of the entries reversed.
   */
  static Array Reverse(CArrRef input, bool preserve_keys = false);

  /**
   * Randomly shuffle the contents of an array.
   */
  static Array Shuffle(CArrRef input);

  /**
   * Return key/keys for random entry/entries in the array.
   */
  static Variant RandomKeys(CArrRef input, int num_req = 1);
  static Variant RandomValues(CArrRef input, int num_req = 1);

  /**
   * Filters elements from the array via the callback.
   */
  typedef bool (*PFUNC_FILTER)(CVarRef value, const void *data);
  static Array Filter(CArrRef input, PFUNC_FILTER filter = NULL,
                      const void *data = NULL);

  /**
   * Removes duplicate values from array.
   */
  static Array Unique(CArrRef input);

  /////////////////////////////////////////////////////////////////////////////
  // Iterations.

  /**
   * Apply a user function to every member of an array.
   */
  typedef void (*PFUNC_WALK)(Variant value, CVarRef key, CVarRef userdata,
                             const void *data);
  static void Walk(Variant input, PFUNC_WALK walk_function, const void *data,
                   bool recursive = false, CVarRef userdata = null_variant);

  /**
   * Applies the callback to the elements in given arrays.
   */
  typedef Variant (*PFUNC_MAP)(CArrRef params, const void *data);
  static Array Map(CArrRef inputs, PFUNC_MAP map_function, const void *data);

  /**
   * Iteratively reduce the array to a single value via the callback.
   */
  typedef Variant (*PFUNC_REDUCE)(CVarRef result, CVarRef operand,
                                  const void *data);
  static Variant Reduce(CArrRef input, PFUNC_REDUCE reduce_function,
                        const void *data, CVarRef initial = null_variant);

  /**
   * Construct scalar arrays from input data.
   */
  static void InitScalarArrays(Array arrs[], int nArrs,
                               const char *scalarArrayData,
                               int scalarArrayDataSize);

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_UTIL_H__
