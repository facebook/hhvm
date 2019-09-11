<?hh // partial

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
<<__Native>>
function clear_static_memoization(?string $cls, ?string $func = null) : bool;

<<__Native>>
function ffp_parse_string_native(string $program): string;

function ffp_parse_string(string $program): array {
  $json = ffp_parse_string_native($program);
  // 2048 is MAX_JSON_DEPTH to avoid making a global constant
  return \json_decode($json, true, 2048);
}

/**
 * Clear __MemoizeLSB data
 *  - if $func is non-null, clear cache for $cls::$func
 *  - if $func is null, clear all LSB memoization caches for $cls
 *
 * Operates on a single class at a time. Clearing the cache for $cls::$func
 * does not clear the cache for $otherClass::$func, for any other class.
 */
<<__Native>>
function clear_lsb_memoization(string $cls, ?string $func = null) : bool;

/**
 * Clear memoization data on object instance
 */
<<__Native>>
function clear_instance_memoization(object $obj) : bool;

/**
 * Attach metadata to the caller's stack frame. The metadata can be retrieved
 * using debug_backtrace(DEBUG_BACKTRACE_PROVIDE_METADATA).
 */
 <<__Native>>
function set_frame_metadata(mixed $metadata): void;

/**
 * Get the total number of requests dispatched since the server started.
 */
<<__Native>>
function get_request_count(): int;

/**
 * Get the number of units that were loaded for this request, filtering for
 * units which ($kind = 0) were compiled in this request, ($kind = 1) were
 * compiled or loaded from the bytecode cache in this request, or ($kind = 2)
 * were compiled, loaded from disk cache, or caused the request to stall waiting
 * for loading to complete in another request. If $kind < 0, only return units
 * which weren't shown to be identical at the bytecode level.
 */
<<__Native>>
function get_compiled_units(int $kind = 0): keyset;

/**
 * Construct a function pointer for the function with $name. The function should
 * be marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_fun(string $name): mixed;

/**
 * Construct a cls_meth pointer for the method $cls::$meth. The method should be
 * a static method marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_class_meth(string $cls, string $meth): mixed;

/**
 * Same as dynamic_fun but can't be used in RepoAuthoritative mode and
 * doesn't raise warnings or errors
 * on functions not marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_fun_force(string $name): mixed;

/**
 * Same as dynamic_class_meth but can't be used in RepoAuthoritative mode
 * and doesn't raise warnings or errors
 * on methods not marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_class_meth_force(string $cls, string $meth): mixed;

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
interface FileAttribute {}

interface TypeParameterAttribute {}

interface TypeConstantAttribute {}

}


namespace HH\Rx {
/**
  * Reactive version of HH\IMemoizeParam.
  * Classes that implement this interface may be passed to
  * serialize_memoize_param() and may be used as params on
  * <<__Memoize>> functions
  */
interface IMemoizeParam extends \HH\IMemoizeParam {
   /**
   * Serialize this object to a string that can be used as a
   * dictionary key to differentiate instances of this class.
   */
  <<__Rx>>
  public function getInstanceKey(): string;
}
}

namespace HH\rqtrace {

type EventStats = shape('duration' => int, 'count' => int);

/**
 * Checks wither rqtrace is enabled for the current request.
 */
<<__Native>>
function is_enabled(): bool;

/**
 * Forcibly enable rqtrace for the current request if it is not already enabled.
 */
<<__Native>>
function force_enable(): void;

/**
 * Return a map of event_name->EventStats for all events which occurred up to
 * the point that this function was called within the current request.
 */
<<__Native>>
function all_request_stats(): mixed; /* darray<string, EventStats>*/

/**
 * Return a map of event_name->EventStats for all events which occurred during
 * previously completed requests at the time this function was called.
 */
<<__Native>>
function all_process_stats(): mixed; /*darray<string, EventStats>*/

/**
 * Return stats for occurences of $event during the current requests up to the
 * call of this function.
 */
<<__Native>>
function request_event_stats(string $event): mixed /* EventStats */;

/**
 * Return stats for all occurences of $event during previously completed
 * requests when this function was called.
 */
<<__Native>>
function process_event_stats(string $event): mixed /* EventStats */;

}

namespace HH\ReifiedGenerics {

  /**
   * Returns the type structure representation of the reified type
   */
  function get_type_structure<reify T>(): mixed {
    return ${'0ReifiedGenerics'}[0];
  }

  /**
   * Returns the name of the class represented by this reified type.
   * If this type does not represent a class, throws an exception
   */
  function get_classname<reify T>(): classname<T> {
    $clsname = idx(namespace\get_type_structure<T>(), 'classname', null);
    if ($clsname is null) {
      throw new \Exception('Trying to get the classname out of a reified type'.
                           ' that does not represent a class');
    }
    return $clsname;
  }

}
