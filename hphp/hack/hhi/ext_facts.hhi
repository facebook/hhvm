<?hh

namespace HH\Facts {

  /**
   * Used to communicate whether a symbol string is the name of a type, function,
   * constant, or type alias.
   *
   * Replicated as `AutoloadMap::KindOf` in `autoload-map.h`
   */
  enum SymbolKind: int {
    K_TYPE = 1;
    K_FUNCTION = 2;
    K_CONSTANT = 3;
    K_MODULE = 4;
  }

  enum TypeKind: string {
    K_CLASS = 'class';
    K_ENUM = 'enum';
    K_INTERFACE = 'interface';
    K_TRAIT = 'trait';
    K_TYPE_ALIAS = 'typeAlias';
  }

  /**
   * The two forms of inheritance supported by Hack.
   */
  enum DeriveKind: string {
    // Includes `implements` and `use`
    K_EXTENDS = 'extends';
    // Includes `require implements`
    K_REQUIRE_EXTENDS = 'require extends';
  }

  type TypeAttributeFilter = shape(
    'name' => classname<\HH\ClassLikeAttribute>,
    'parameters' => dict<int, dynamic>,
  );

  type DeriveFilters = shape(
    ?'kind' => keyset<TypeKind>,
    ?'derive_kind' => keyset<DeriveKind>,
    ?'attributes' => vec<TypeAttributeFilter>,
  );

  /**
   * True iff native facts are available
   *
   * If this returns false, any other operations in the HH\Facts namespace will
   * throw InvalidOperationException if called.
   */
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
  function db_path(string $root)[]: ?string;

  /**
   * Return the schema version in use by this hhvm binary.
   */
  function schema_version()[]: int;

  /**
   * Blocks until Facts is synchronized as of when the call was intiated.
   */
  function sync(): void;

  /**
   * Return the only path defining a given symbol.
   *
   * Return `null` if the symbol is not defined, or is defined in more than one
   * place.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_to_path(string $type_name)[]: ?string;
  function function_to_path(string $function_name)[]: ?string;
  function constant_to_path(string $constant_name)[]: ?string;
  function type_alias_to_path(string $type_alias_name)[]: ?string;
  function type_or_type_alias_to_path(string $type_name)[]: ?string;

  /**
   * Return all the symbols defined in the given path.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function path_to_modules(string $path)[]: vec<string>;
  function path_to_types(string $path)[]: vec<classname<nonnull>>;
  function path_to_functions(string $path)[]: vec<string>;
  function path_to_constants(string $path)[]: vec<string>;
  function path_to_type_aliases(string $path)[]: vec<string>;

  /**
   * Resolve a string into a classname that's properly capitalized and properly
   * typed.
   *
   * Return `null` if the classname does not exist in the codebase, even with
   * different capitalization.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_name(string $classname)[]: ?classname<nonnull>;

  /**
   * Return a string enum representing whether the given type is, for example, a
   * class, interface, or trait.
   *
   * If the given type doesn't have a unique definition or isn't a
   * `classname<mixed>`, return `null`.
   */
  function kind(classname<mixed> $type)[]: ?TypeKind;

  /**
   * True iff the given type cannot be constructed.
   */
  function is_abstract(classname<mixed> $type)[]: bool;

  /**
   * True iff the given type cannot be inherited.
   */
  function is_final(classname<mixed> $type)[]: bool;

  /**
   * Get all types which extend, implement, or use the given base type.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function subtypes<T>(
    classname<T> $base_type,
    ?DeriveFilters $filters = null,
  )[]: vec<classname<T>>;

  /**
   * Return all types which transitively extend, implement, or use the given
   * base type, as `(name, path, kind, abstract)` tuples.
   *
   * The 'kind' and 'derive_kind' filters passed in determine which relationships
   * and types we look at while traversing the inheritance graph. So if you
   * filter traits out, we'll exclude classes which are only related because
   * they `use` a trait which `implements` the interface you passed in.
   *
   * The 'attributes' filters passed in will be applied to the final list of
   * transitive subtypes. So if you look for types with the `<<Oncalls('team')>>`
   * attribute, we'll only filter the final list of subtypes, instead of ignoring
   * all types that don't have the given attribute.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function transitive_subtypes<T>(
    classname<T> $base_type,
    ?DeriveFilters $filters = null,
  )[]: vec<(classname<T>, string, TypeKind, bool)>;

  /**
   * Get all types which the given type extends, implements, or uses.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function supertypes(
    classname<mixed> $derived_type,
    ?DeriveFilters $filters = null,
  )[]: vec<classname<mixed>>;

  /**
   * Get all types matching the given filters.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function types_with_attribute(
    classname<\HH\ClassLikeAttribute> $attribute,
  )[]: vec<classname<mixed>>;

  /**
   * Get all type aliases matching the given filters.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function type_aliases_with_attribute(
    classname<\HH\TypeAliasAttribute> $attribute,
  )[]: vec<string>;

  /**
   * Get all methods matching the given filters.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   * Throws a RuntimeException if querying for an attribute that's not listed
   *   in the `Autoload.IndexedMethodAttributes` setting in this repo's
   *   `.hhvmconfig.hdf` file.
   */
  function methods_with_attribute(
    classname<\HH\MethodAttribute> $attribute,
  )[]: vec<(classname<nonnull>, string)>;

  /**
   * Get all files matching the given filters.
   *
   * Throws InvalidOperationException if Facts is not enabled.
   */
  function files_with_attribute(
    classname<\HH\FileAttribute> $attribute,
  )[]: vec<string>;

  /**
   * Get all attributes on the given type.
   *
   * Return an empty vec if the given type doesn't exist or is defined in more
   * than one file.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_attributes(
    classname<mixed> $type,
  )[]: vec<classname<\HH\ClassLikeAttribute>>;

  /**
   * Get all attributes on the given type alias.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_alias_attributes(
    string $type_alias,
  )[]: vec<classname<\HH\TypeAliasAttribute>>;

  /**
   * Get all attributes on the given method.
   *
   * Return an empty vec if the method does not exist.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function method_attributes(
    classname<mixed> $type,
    string $method,
  )[]: vec<classname<\HH\MethodAttribute>>;

  /**
   * Get all attributes on the given file.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function file_attributes(string $file)[]: vec<classname<\HH\FileAttribute>>;

  /**
   * Get all parameters for the given attribute on the given type.
   *
   * Return an empty vec if the given type doesn't exist or doesn't have the
   * given attribute.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_attribute_parameters(
    classname<mixed> $type,
    classname<\HH\ClassLikeAttribute> $attribute,
  )[]: vec<dynamic>;

  /**
   * Get all parameters for the given attribute on the given type alias.
   *
   * Return an empty vec if the type alias doesn't exist or doesn't have the
   * given attribute.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function type_alias_attribute_parameters(
    string $type_alias,
    classname<\HH\TypeAliasAttribute> $attribute,
  )[]: vec<dynamic>;

  /**
   * Get all parameters for the given attribute on the given method.
   *
   * Return an empty vec if the method doesn't exist or doesn't have the
   * given attribute.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function method_attribute_parameters(
    classname<mixed> $type,
    string $method,
    classname<\HH\MethodAttribute> $attribute,
  )[]: vec<dynamic>;

  /**
   * Get all parameters for the given attribute on the given file.
   *
   * Return an empty vec if the given file doesn't exist or doesn't have the
   * given attribute.
   *
   * Throw InvalidOperationException if Facts is not enabled.
   */
  function file_attribute_parameters(
    string $file,
    classname<\HH\FileAttribute> $attribute,
  )[]: vec<dynamic>;

  /**
   * Return all symbols defined in the repo, as a dict mapping each symbol
   * name to the path where the symbol lives in the repo.
   *
   * If a symbol is defined in more than one path, one of the paths defining the
   * symbol will be chosen in an unspecified manner.
   */
  function all_modules()[]: dict<string, string>;
  function all_types()[]: dict<classname<mixed>, string>;
  function all_functions()[]: dict<string, string>;
  function all_constants()[]: dict<string, string>;
  function all_type_aliases()[]: dict<string, string>;

  type AttributeData = shape(
    'name' => string,
    'args' => vec<?arraykey>,
  );

  type MethodData = shape(
    'name' => string,
    'attributes' => vec<AttributeData>,
  );

  type TypeData = shape(
    'name' => string,
    'kind' => TypeKind,
    'flags' => int,
    'baseTypes' => vec<string>,
    'requireExtends' => vec<string>,
    'requireImplements' => vec<string>,
    'attributes' => vec<AttributeData>,
    'methods' => vec<MethodData>,
  );

  type ModuleData = shape(
    'name' => string,
  );

  type FileData = shape(
    'modules' => vec<string>,
    'types' => vec<TypeData>,
    'functions' => vec<string>,
    'constants' => vec<string>,
    'attributes' => vec<AttributeData>,
    'sha1sum' => string,
  );

  /**
   * For each path/hash pair in `$pathsAndHashes`, parse the file on the
   * filesystem, or lookup the file with the given SHA1 hash, and return a dict
   * mapping each path to its contents.
   *
   * Each given path should be relative to the given `$root`.
   */
  function extract(
    vec<(string, ?string)> $pathsAndHashes,
    ?string $root = null,
  ): dict<string, ?FileData>;

} // namespace HH\Facts
