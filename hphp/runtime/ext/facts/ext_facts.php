<?hh

namespace HH\Facts {

/**
 * Used to communicate whether a symbol string is the name of a type, function,
 * constant, or type alias.
 *
 * Replicated as `AutoloadMap::KindOf` in `autoload-map.h`
 */
enum SymbolKind : int {
  K_TYPE = 1;
  K_FUNCTION = 2;
  K_CONSTANT = 3;
  K_MODULE = 4;
}

/**
 * These are the different kinds of types that Facts concerns itself with.
 * These values are replicated in ext_facts.h.
 */
enum TypeKind : string {
  K_CLASS = 'class';
  K_ENUM = 'enum';
  K_INTERFACE = 'interface';
  K_TRAIT = 'trait';
  K_TYPE_ALIAS = 'typeAlias';
}

/**
 * Type flags include abstract and final. Empty means no flags i.e.
 * `class MyClass {}`
 */
enum TypeFlag : string {
  K_EMPTY = 'empty';
  K_ABSTRACT = 'abstract';
  K_FINAL = 'final';
}

/**
 * The two forms of inheritance supported by Hack.
 */
enum DeriveKind : string {
  // Includes `implements` and `use`
  K_EXTENDS = 'extends';
  // Includes `require implements`
  K_REQUIRE_EXTENDS = 'require extends';
}

type TypeAttributeFilter = shape(
  'name' => classname<\HH\ClassLikeAttribute>,
  'parameters' => dict<int, dynamic>,
);

/**
 * Different filters for for type queries, not just for derived types but
 * also for base types. These filters default to 'include everything' if they
 * are omitted, not 'include nothing'. See the enums for what type of thing
 * goes in each field.
 */
type DeriveFilters = shape(
  ?'kind' => keyset<TypeKind>,
  ?'derive_kind' => keyset<DeriveKind>,
  ?'flags' => vec<TypeFlag>,
);

/**
 * True iff native facts are available
 *
 * If this returns false, any other operations in the HH\Facts namespace will
 * throw InvalidOperationException if called.
 */
<<__Native>>
function enabled()[]: bool;

/**
 * Return the DB path corresponding to the given directory of Hack code.
 *
 * The given directory must be a valid path containing a `.hhvmconfig.hdf`
 * file at its root, and this `.hhvmconfig.hdf` file must contain either the
 * `Autoload.TrustedDBPath` or the `Autoload.Query` setting. Otherwise, this
 * function will return `null`.
 *
 * If the `Autoload.TrustedDBPath` setting points to a valid path, this
 * function will just return that path.
 *
 * Otherwise, if the `Autoload.Query` setting exists, this function calculates
 * the DB's location for that repo based on the query, path, version, and Unix
 * user.
 */
<<__Native>>
function db_path(string $root)[]: ?string;

/**
 * Return the schema version in use by this hhvm binary.
 */
<<__Native>>
function schema_version()[]: int;

/**
 * Blocks until Facts is synchronized as of when the call was intiated.
 */
<<__Native>>
function sync(): void;

/**
 * Return the only path defining a given symbol.
 *
 * Return `null` if the symbol is not defined, or is defined in more than one
 * place.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function module_to_path(string $module_name)[]: ?string;
<<__Native>>
function type_to_path(string $type_name)[]: ?string;
<<__Native>>
function function_to_path(string $function_name)[]: ?string;
<<__Native>>
function constant_to_path(string $constant_name)[]: ?string;
<<__Native>>
function type_alias_to_path(string $type_alias_name)[]: ?string;
<<__Native>>
function type_or_type_alias_to_path(string $type_name)[]: ?string;

/**
 * Return all the symbols defined in the given path.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function path_to_modules(string $path)[]: vec<string>;
<<__Native>>
function path_to_types(string $path)[]: vec<string>;
<<__Native>>
function path_to_functions(string $path)[]: vec<string>;
<<__Native>>
function path_to_constants(string $path)[]: vec<string>;
<<__Native>>
function path_to_type_aliases(string $path)[]: vec<string>;

/**
 * Return the module the file is a member of, if any.
 */
<<__Native>>
function path_to_module_membership(string $path)[]: ?string;

/**
 * Return the package the file is a member of, if any.
 */
<<__Native>>
function path_to_package(string $path)[]: ?string;

/**
 * Return the sha1 of the path, if any.
 */
<<__Native>>
function sha1(string $path)[]: ?string;

/**
 * Resolve a string into a classname that's properly capitalized and
 * properly typed.
 *
 * Return `null` if the classname does not exist in the codebase, even with
 * different capitalization.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_name(string $type_name)[]: ?string;

/**
 * Return a string enum representing whether the given type is, for example, a
 * class, interface, or trait.
 *
 * If the given type doesn't have a unique definition or isn't a
 * classname<nonnull>, return `null`.
 */
<<__Native>>
function kind(
  /* classname<nonnull> */ string $type,
)[]: /* ?TypeKind */ mixed;

/**
 * True iff the given type cannot be constructed.
 */
<<__Native>>
function is_abstract(
  /* classname<nonnull> */ string $type,
)[]: bool;

/**
 * validate facts sql database
 */
<<__Native>>
function validate(vec<string> $types_to_ignore = vec[]): void;

/**
 * True iff the given type cannot be inherited.
 */
<<__Native>>
function is_final(
  /* classname<nonnull> */ string $type,
)[]: bool;

/**
 * Get all types which extend, implement, or use the given base type.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function subtypes(
  class_or_classname<mixed> $base_type,
  /* DeriveFilters */ ?darray<string, dynamic> $filters = null,
)[]: vec<class<mixed>>;

/**
 * Get the transitive types which extend, implement, or use the given base type.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function transitive_subtypes(
  class_or_classname<mixed> $base_type,
  /* DeriveFilters */ ?darray<string, dynamic> $filters = null,
  bool $include_interface_require_extends = false,
 )[]: vec<class<mixed>>;

/**
 * Get all types which the given type extends, implements, or uses.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function supertypes(
  class_or_classname<mixed> $derived_type,
  /* DeriveFilters */ ?darray<string, dynamic> $filters = null,
)[]: vec<class<mixed>>;

/**
 * Get all types matching the given filters.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function types_with_attribute(
  /* classname<\HH\ClassLikeAttribute> */ string $attribute,
)[]: vec<string>; /* vec<classname<nonnull>> */

/**
 * Get all type aliases matching the given filters.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_aliases_with_attribute(
  /* classname<\HH\TypeAliasAttribute> */ string $attribute,
)[]: vec<string>;

/**
 * Get all methods matching the given filters.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 * Throws a RuntimeException if querying for an attribute that's not listed
 *   in the `Autoload.IndexedMethodAttributes` setting in this repo's
 *   `.hhvmconfig.hdf` file.
 */
<<__Native>>
function methods_with_attribute(
  /* classname<\HH\MethodAttribute> */ string $attribute,
)[]: vec<(classname<nonnull>, string)>;

/**
 * Get all files matching the given filters.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function files_with_attribute(
  /* classname<\HH\FileAttribute> */ string $attribute,
)[]: vec<string>;

/**
 * Get all files matching the given filters.
 * Only bool, int, string, dict, vec, and keyset are supported
 * as value types to query for. Unsupported types will be
 * coerced to null.
 *
 * Throws InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function files_with_attribute_and_any_value(
  /* classname<\HH\FileAttribute> */ string $attribute,
  dynamic $value,
)[]: vec<string>;

/**
 * Get all files with the following attribute, including the argument list for
 * that attribute if it exists.  Because each item in the returned list is a
 * tuple including path and attr arg, if a file has more than one attr arg
 * for the given attr, that file will appear in the return list more than once.
 * If there is no argument for this attr, the second arg will be null.
 */
<<__Native>>
function files_and_attr_args_with_attribute(
  /* classname<\HH\FileAttribute> */ string $attribute,
)[]: vec<(string, ?string)>;

/**
 * Get all attributes on the given type.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_attributes(
  /* classname<nonnull> */ string $type,
)[]: vec<string>; /* vec<classname<\HH\ClassLikeAttribute>> */

/**
 * Get all attributes on the given type alias.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_alias_attributes(
  string $type_alias,
)[]: vec<string>; /* vec<classname<\HH\TypeAliasAttribute>> */

/**
 * Get all attributes on the given method.
 *
 * Return an empty vec if the method doesn't exist.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function method_attributes(
  /* classname<nonnull> */ string $type,
  string $method,
)[]: vec<string>; /* vec<classname<\HH\MethodAttribute>> */

/**
 * Get all attributes on the given file.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function file_attributes(
  string $file,
)[]: vec<string>; /* vec<classname<\HH\FileAttribute>> */

/**
 * Get all parameters for the given attribute on the given type.
 *
 * Return an empty vec if the type doesn't exist or doesn't have the given
 * attribute.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_attribute_parameters(
  /* classname<nonnull> */ string $type,
  /* classname<\HH\ClassLikeAttribute> */ string $attribute,
)[]: vec<dynamic>;

/**
 * Get all parameters for the given attribute on the given type alias.
 *
 * Return an empty vec if the type alias doesn't exist or doesn't have the
 * given attribute.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function type_alias_attribute_parameters(
  string $type_alias,
  /* classname<\HH\TypeAliasAttribute> */ string $attribute,
)[]: vec<dynamic>;

/**
 * Get all parameters for the given attribute on the given method.
 *
 * Return an empty vec if the method doesn't exist or doesn't have the
 * given attribute.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function method_attribute_parameters(
  /* classname<nonnull> */ string $type,
  string $method,
  /* classname<\HH\MethodAttribute> */ string $attribute,
)[]: vec<dynamic>;

/**
 * Get all parameters for the given attribute on the given file.
 *
 * Return an empty vec if the file doesn't exist or doesn't have the given
 * attribute.
 *
 * Throw InvalidOperationException if Facts is not enabled.
 */
<<__Native>>
function file_attribute_parameters(
  string $file,
  /* classname<\HH\FileAttribute> */ string $attribute,
)[]: vec<dynamic>;

} // namespace HH\Facts
