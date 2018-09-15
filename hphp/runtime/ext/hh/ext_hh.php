<?hh

namespace HH {

/**
  * Classes that implement this interface may be passed to
  * serialize_memoize_param() and may be used as params on
  * <<__Memoize>> functions
  */
interface IMemoizeParam {
   /**
   * Serialize this object to a string that can be used as a
   * dictionary key to differentiate instances of this class.
   */
  public function getInstanceKey(): string;
}

/** Specify a map containing autoload data.
 *
 * The map has the form:
 *
 * ```
 *  array('class'    => array('cls' => 'cls_file.php', ...),
 *        'function' => array('fun' => 'fun_file.php', ...),
 *        'constant' => array('con' => 'con_file.php', ...),
 *        'type'     => array('type' => 'type_file.php', ...),
 *        'failure'  => callable);
 * ```
 *
 *  If the 'failure' element exists, it will be called if the
 *  lookup in the map fails, or the file cant be included. It
 *  takes a kind ('class', 'function' or 'constant') and the
 *  name of the entity we're trying to autoload.
 *
 * If $root is non empty, it is prepended to every filename
 * (so will typically need to end with '/').
 *
 * @param map The autoload map.
 * @param root Root to be prepended to all paths in the map.
 *
 * @return Boolean TRUE if successful, FALSE otherwise.
 */
<<__Native>>
function autoload_set_paths(mixed $map, string $root): bool;

/**
  * Returns whether the (php) file could be included (eg if its been compiled
  * into the binary). This is useful when you don't have a filesystem
  * (RepoAuthoritative mode) but still want to know if including a file will
  * work.
  */
<<__Native>>
function could_include(string $file) : bool;

/**
  * Takes an argument to a function marked with <<__Memoize>> and serializes it
  * to a string usable as a unique cache key. This works with all builtin types
  * and with objects that implement the HH\IMemoizeParam interface
  */
<<__Native>>
function serialize_memoize_param(mixed $param): arraykey;

/**
 * Clear memoization data
 *  - if $cls is non-null, clear memoziation cache for $cls::$func,
 *    or for all static memoized methods if $func is null
 *  - if $cls is null, clear memoization cache for $func
 */
<<__Native, __ParamCoerceModeFalse>>
function clear_static_memoization(?string $cls, ?string $func = null) : bool;

/**
 * Clear __MemoizeLSB data
 *  - if $func is non-null, clear cache for $cls::$func
 *  - if $func is null, clear all LSB memoization caches for $cls
 *
 * Operates on a single class at a time. Clearing the cache for $cls::$func
 * does not clear the cache for $otherClass::$func, for any other class.
 */
<<__Native, __ParamCoerceModeFalse>>
function clear_lsb_memoization(string $cls, ?string $func = null) : bool;

/**
 * Clear memoization data on object instance
 */
<<__Native, __ParamCoerceModeFalse>>
function clear_instance_memoization(object $obj) : bool;

/**
 * Attach metadata to the caller's stack frame. The metadata can be retrieved
 * using debug_backtrace(DEBUG_BACKTRACE_PROVIDE_METADATA).
 */
<<__Native("WritesCallerFrame")>>
function set_frame_metadata(mixed $metadata): void;

// class-like
interface ClassLikeAttribute {}
interface ClassAttribute extends ClassLikeAttribute {}
interface EnumAttribute extends ClassLikeAttribute {}

interface TypeAliasAttribute {}

// function-like
interface FunctionAttribute {}
interface MethodAttribute {}

// properties
interface PropertyAttribute {}
interface InstancePropertyAttribute extends PropertyAttribute {}
interface StaticPropertyAttribute extends PropertyAttribute {}

interface ParameterAttribute {}

}
