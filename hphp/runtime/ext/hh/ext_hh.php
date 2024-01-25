<?hh

namespace HH {

/**
  * Classes that implement this interface may be passed to
  * serialize_memoize_param() and may be used as params on
  * <<__Memoize>> functions
  */
interface IMemoizeParam {
  abstract const ctx CMemoParam = [defaults];
   /**
   * Serialize this object to a string that can be used as a
   * dictionary key to differentiate instances of this class.
   */
  public function getInstanceKey(): string;
}

/*
 * Unsafe marker interface for an IMemoizeParam used to override
 * typechecker check that IMemoizeParam objects are only used as parameters
 * on functions that have access to globals.
 * Singleton classes are allowed to be passed into pure memoized functions because there can't
 * be a data leak associated with them.
 * TODO: enforce SingletonMemoizeParam in some way in runtime/typechecker
*/
interface UNSAFESingletonMemoizeParam extends IMemoizeParam {
  public function getInstanceKey(): string;
}

/**
 * Return true if we're using a native autoloader. If this returns
 * false, HHVM was not configured to enable autoloading; code can
 * still access symbols in the same files, or included by require_once().
 */
<<__Native>>
function autoload_is_native(): bool;

/**
 * Get the path which uniquely defines the given symbol.
 *
 * Returns an absolute canonical path with all symlinks dereferenced.
 *
 * Throws InvalidOperationException if native autoloading is disabled.
 */
<<__Native>>
function autoload_type_to_path(string $type): ?string;
<<__Native>>
function autoload_function_to_path(string $function): ?string;
<<__Native>>
function autoload_constant_to_path(string $constant): ?string;
<<__Native>>
function autoload_module_to_path(string $module): ?string;
<<__Native>>
function autoload_type_alias_to_path(string $type_alias): ?string;
<<__Native>>
function autoload_type_or_type_alias_to_path(string $type): ?string;

/**
 * Get the types defined in the given path.
 *
 * The path may be relative to the repo root or absolute. But this function
 * will not dereference symlinks for you, so providing a path with symlinks
 * may cause this function to return an empty vec when you expected results.
 *
 * Throws InvalidOperationException if native autoloading is disabled.
 */
<<__Native>>
function autoload_path_to_types(string $path): vec<classname<mixed>>;
<<__Native>>
function autoload_path_to_functions(string $path): vec<string>;
<<__Native>>
function autoload_path_to_constants(string $path): vec<string>;
<<__Native>>
function autoload_path_to_modules(string $path): vec<string>;
<<__Native>>
function autoload_path_to_type_aliases(string $path): vec<string>;

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
<<__Native("NoRecording")>>
function clear_static_memoization(?string $cls, ?string $func = null) : bool;

<<__Native>>
function ffp_parse_string_native(string $program)[]: string;

newtype ParseTree = darray;

function ffp_parse_string(string $program)[]: ParseTree {
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
<<__Native("NoRecording")>>
function clear_lsb_memoization(string $cls, ?string $func = null) : bool;

/**
 * Clear memoization data on object instance
 */
<<__Native("NoRecording")>>
function clear_instance_memoization(\HH\object $obj) : bool;

/**
 * Attach metadata to the caller's stack frame. The metadata can be retrieved
 * using debug_backtrace(DEBUG_BACKTRACE_PROVIDE_METADATA).
 */
 <<__Native>>
function set_frame_metadata(mixed $metadata)[]: void;

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
 * Prefetch units corresponding to the given sets of paths, before
 * they are actually needed.
 *
 * - If $hint is true, this request is non-binding. That is, the
 *   runtime may attempt to asynchronously prefetch the units in the
 *   background, or maybe not do anything at all. The units may not
 *   (and probably won't be) loaded when this function returns.
 *
 * - If $hint is false, the runtime will always attempt to prefetch
 *   the units and will block until they have been. Therefore when
 *   this function returns, the units have been loaded. Note:
 *   depending on configuration, this may be slower than a non-binding
 *   request.
 */
<<__Native>>
function prefetch_units(keyset<string> $paths, bool $hint): void;

/**
 * Construct a function pointer for the function with $name. The function should
 * be marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_fun(string $name)[]: mixed;

/**
 * Construct a cls_meth pointer for the method $cls::$meth. The method should be
 * a static method marked __DynamicallyCallable.
 */
<<__Native("NoRecording")>>
function dynamic_class_meth(string $cls, string $meth)[]: mixed;

/**
 * Same as dynamic_fun but can't be used in RepoAuthoritative mode and
 * doesn't raise warnings or errors
 * on functions not marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_fun_force(string $name)[]: mixed;

/**
 * Same as dynamic_class_meth but can't be used in RepoAuthoritative mode
 * and doesn't raise warnings or errors
 * on methods not marked __DynamicallyCallable.
 */
<<__Native>>
function dynamic_class_meth_force(string $cls, string $meth)[]: mixed;

/**
 * Creates a LazyClass pointer from input $classname. It does not eagerly
 * verify that the $classname is in fact a valid class.
 */
<<__Native>>
function classname_from_string_unsafe(string $classname)[]: mixed;

<<__Native>>
function class_to_classname(readonly classname<mixed> $cn)[]: classname<mixed>;

/**
 * Begin collecting code coverage on all subsequent calls into files in $files
 * during this request.
 *
 * The requst must be executing in non-RepoAuthoritative mode and the server
 * must be configured with Eval.EnablePerFileCoverage = true.
 *
 * @param $files a list of paths to collect coverage from
 */
<<__Native>>
function enable_per_file_coverage(keyset<string> $files): void;

/**
 * Stop collecting coverage on all subsequent calls into files in $files during
 * this request.
 *
 * @param $files a list of paths to stop collecting coverage from
 */
<<__Native>>
function disable_per_file_coverage(keyset<string> $files): void;

/**
 * Returns a list of files for which coverage has been enabled in this request.
 */
<<__Native>>
function get_files_with_coverage(): keyset<string>;

/**
 * Extract coverage data for the file at path $file. The returned vector
 * contains a list of line numbers that were seen at least once while coverage
 * was enablgc_enabled for the file.
 *
 * @return a list of covered line numbers
 */
<<__Native>>
function get_coverage_for_file(string $file): vec<int>;

/**
 * Clear all coverage data for the file at path $file. Continue collecting
 * coverage for that file.
 *
 * @param $file the path of the file to clear coverage data for
 */
<<__Native>>
function clear_coverage_for_file(string $file): void;

/**
 * Disable all coverage. Stops collecting code coverage on any file in the
 * current request.
 */
function disable_all_coverage(): void {
  disable_per_file_coverage(get_files_with_coverage());
}

/**
 * Collect coverage data for all files covered in this request as a map from
 * filepath to a list of covered lines.
 *
 * @return a map of filepath -> line vector
 */
function get_all_coverage_data(): dict<string, vec<int>> {
  $ret = dict[];
  foreach (get_files_with_coverage() as $file) {
    $ret[$file] = get_coverage_for_file($file);
  }
  return $ret;
}

/**
 * Clear all coverage data collected during this requests, continue to collect
 * new coverage data.
 */
function clear_all_coverage_data(): void {
  foreach (get_files_with_coverage() as $file) {
    clear_coverage_for_file($file);
  }
}

/**
 * Return a vector of lines known to be executable in $file. WARNING: there is
 * no guarantee that these lines will be seen when running code-coverage mode.
 * This API is purely a heuristic/best effort approximation of executable lines.
 */
<<__Native>>
function get_executable_lines(string $file): mixed;

/**
 * Gets an integer which when combined with the thread ID (hphp_get_thread_id)
 * and process id (posix_getpid), can be used to uniquely identify log lines
 * generated by the current request.
 *
 * @return int - the per-thread request id used by logger to identify the
 *   current request.
 *
 */
<<__Native>>
function hphp_get_logger_request_id(): int;

/**
 * Enables recording of called functions in the current request. The server must
 * not be in repo authoritative mode and Eval.EnableFuncCoverage must be set,
 * additionally coverage must not already have been enabled in the current
 * request (or must have been disabled via a call to collect_function_coverage).
 */
<<__Native>>
function enable_function_coverage(): void;

/**
 * Returns a dict keyed on functions called since enable_function_coverage was
 * last called, with the paths of the files defining them as values,
 * and disables collection of further function coverage.
 *
 * The enable_function_coverage function must have been called prior to this
 * function.
 */
<<__Native>>
function collect_function_coverage(): dict<string, string>;

/**
 * Return all package information from PACKAGES.toml
 */
<<__Native>>
function get_all_packages(
): dict<string, shape('uses' => vec<string>, 'includes' => vec<string>)>;

/**
 * Return all deploment information from PACKAGES.toml
 */
<<__Native>>
function get_all_deployments(
): dict<string, shape('packages' => vec<string>, 'domains' => vec<string>)>;

/*
 * Returns whether a package named $name exist in the current deployment.
 */
<<__Native>>
function package_exists(string $name): bool;

} // HH

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
  function get_type_structure<reify T>()[]: mixed {
    return \__systemlib_reified_generics()[0];
  }

  /**
   * Returns the name of the class represented by this reified type.
   * If this type does not represent a class, throws an exception
   */
  function get_classname<reify T>()[]: classname<T> {
    $clsname = idx(namespace\get_type_structure<T>(), 'classname', null);
    if ($clsname is null) {
      throw new \Exception('Trying to get the classname out of a reified type'.
                           ' that does not represent a class');
    }
    return $clsname;
  }

}

namespace HH\TypeStructure {

<<__Native>>
function get_kind(dict<string, mixed> $ts): int;

<<__Native>>
function get_nullable(dict<string, mixed> $ts): bool;

<<__Native>>
function get_soft(dict<string, mixed> $ts): bool;

<<__Native>>
function get_opaque(dict<string, mixed> $ts): bool;

<<__Native>>
function get_optional_shape_field(dict<string, mixed> $ts): bool;

<<__Native>>
function get_alias(dict<string, mixed> $ts): string;

<<__Native>>
function get_typevars(dict<string, mixed> $ts): string;

<<__Native>>
function get_typevar_types(dict<string, mixed> $ts): dict<string, mixed>;

<<__Native>>
function get_fields(dict<string, mixed> $ts): dict<string, mixed>;

<<__Native>>
function get_allows_unknown_fields(dict<string, mixed> $ts): bool;

<<__Native>>
function get_elem_types(dict<string, mixed> $ts): vec<mixed>;

<<__Native>>
function get_param_types(dict<string, mixed> $ts): vec<mixed>;

<<__Native>>
function get_return_type(dict<string, mixed> $ts): dict<string, mixed>;

<<__Native>>
function get_variadic_type(dict<string, mixed> $ts): dict<string, mixed>;

<<__Native>>
function get_name(dict<string, mixed> $ts): string;

<<__Native>>
function get_generic_types(dict<string, mixed> $ts): vec<mixed>;

<<__Native>>
function get_classname(dict<string, mixed> $ts): string;

<<__Native>>
function get_exact(dict<string, mixed> $ts): bool;

}

namespace __SystemLib {

<<__Native>>
function is_dynamically_callable_inst_method(
    string $class,
    string $method
)[]: bool;

<<__Native>>
function check_dynamically_callable_inst_method(
    string $class,
    string $method
)[]: void;

<<__Native>>
function reflection_class_get_name(
    mixed $class,
)[]: string;

<<__Native>>
function reflection_class_is_abstract(
    mixed $class,
)[]: bool;

<<__Native>>
function reflection_class_is_final(
    mixed $class,
)[]: bool;

<<__Native>>
function reflection_class_is_interface(
    mixed $class,
)[]: bool;


}
