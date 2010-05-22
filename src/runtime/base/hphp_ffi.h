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

#ifndef __HPHP_FFI_H__
#define __HPHP_FFI_H__

///////////////////////////////////////////////////////////////////////////////
namespace HPHP {
  class Variant;
  class ArrayData;
  class StringData;
  class ObjectData;
  class ExecutionContext;
}

extern "C" {

  /**
   * Decreases the ref count of an HPHP array. Called when the foreign
   * reference is destructed.
   */
  void hphp_ffi_ArrayData_decRef(HPHP::ArrayData *p);
  /**
   * Decreases the ref count of an HPHP string.
   */
  void hphp_ffi_StringData_decRef(HPHP::StringData *p);
  /**
   * Decreases the ref count of an HPHP object.
   */
  void hphp_ffi_ObjectData_decRef(HPHP::ObjectData *p);

  /**
   * Creates an HPHP variant from the foreign language.
   *
   * @param t   encodes the type of the variant
   * @param v   foreign value from which the HPHP variant is built
   * @param len length of the char array used to build the HPHP string
   * @return    pointer to the HPHP variant
   */
  HPHP::Variant *hphp_ffi_buildVariant(int t, void* v, int len);

  /**
   * Deletes an HPHP variant created from the foreign language.
   */
  void hphp_ffi_freeVariant(HPHP::Variant *v);

  /**
   * Sets a <key, value> pair in an HPHP variant. Fails for primitive types.
   */
  void hphp_ffi_addMapItem(HPHP::Variant *map, HPHP::Variant *key,
                           HPHP::Variant *val);

  /**
   * Gets the value corresponding to a key.
   */
  int hphp_ffi_getMapItem(void **result, HPHP::Variant *map,
                          HPHP::Variant *key);

  /**
   * Returns the position of the first valid element in an HPHP array,
   * -1 if the array is empty.
   */
  long long hphp_ffi_iter_begin(HPHP::ArrayData *arr);
  /**
   * Returns the position of the element after the given position,
   * -1 if out of the range.
   */
  long long hphp_ffi_iter_advance(HPHP::ArrayData *arr, long long pos);
  /**
   * Returns the position of the last valid element in an HPHP array,
   * -1 if the array is empty.
   */
  long long hphp_ffi_iter_end(HPHP::ArrayData *arr);
  /**
   * Checks whether the given position is the invalid one, i.e., -1.
   */
  int hphp_ffi_iter_invalid(long long pos);
  /**
   * Gets the key at the given position, and exports it as a variant.
   *
   * @param arr pointer to the HPHP array
   * @param pos position of the key
   * @param res pointer to the retrieved key
   * @return    type of the key
   */
  int hphp_ffi_iter_getKey(HPHP::ArrayData *arr, long long pos, void** res);
  /**
   * Gets the value at the given position, and exports it as a variant.
   */
  int hphp_ffi_iter_getValue(HPHP::ArrayData *arr, long long pos, void** res);

  /**
   * Retrieves the literal string from an HPHP string.
   *
   * @param sd   pointer to an HPHP string
   * @param data pointer to the literal string
   * @return     length of the string
   */
  int hphp_ffi_string_data(HPHP::StringData *sd, const char** data);

  /**
   * Dynamically includes a file with the given name.
   */
  void hphp_ffi_include_file(const char* file);

  /**
   * Invokes a function, and exports the result.
   *
   * @param  ret  pointer to store the result
   * @param  func function name
   * @param  args array of the arguments
   * @return      type of the exported value
   */
  int hphp_ffi_invoke_function(void** ret, const char* func,
                               HPHP::ArrayData* args);

  /**
   * Invokes a static method, and exports the result.
   */
  int hphp_ffi_invoke_static_method(void** ret, const char* cls,
                                    const char* func, HPHP::ArrayData* args);

  /**
   * Invokes an object method, and exports the result.
   */
  int hphp_ffi_invoke_object_method(void** ret, HPHP::ObjectData* receiver,
                                    const char* func, HPHP::ArrayData* args);

  /**
   * Creates an object, and exports the result.
   *
   * @param  ret  pointer to store the result
   * @param  cls  class name
   * @param  args array of the arguments
   * @return      type of the exported value
   */
  int hphp_ffi_create_object(void **ret, const char *cls,
                             HPHP::ArrayData *args);

  /**
   * Gets/sets a global variable.
   */
  int hphp_ffi_get_global(void **ret, const char *name);
  void hphp_ffi_set_global(const char *name, HPHP::Variant *value);

  /**
   * Initializes static constants and runtime options.
   */
  void hphp_ffi_init();

  /**
   * Initializes global variables.
   */
  void hphp_ffi_init_globals();

  /**
   * Frees global variables.
   */
  void hphp_ffi_free_globals();

  /**
   * Inits an Hphp session.
   */
  void hphp_ffi_session_init();

  /**
   * Exits an Hphp session.
   */
  void hphp_ffi_session_exit();

  /**
   * Creates an execution context, and returns the pointer.
   */
  HPHP::ExecutionContext *hphp_ffi_context_init();

  /**
   * Finishes and releases an execution context.
   */
  void hphp_ffi_context_exit(HPHP::ExecutionContext *context);
}

/**
 * Helper function to export data from HPHP to the foreign language.
 *
 * For primitive data, the value is directly exported; for a literal string,
 * the pointer to the string is exported; for an HPHP string, array, or object,
 * the pointer is exported, with the ref count increased by 1.
 *
 * @param  v      const reference to the HPHP variant data to export
 * @param  result pointer for storing the exported data
 * @return        type of the exported value
 */
int hphp_ffi_exportVariant(const HPHP::Variant &v, void** result);

///////////////////////////////////////////////////////////////////////////////

#endif /* __HPHP_FFI_H__ */
