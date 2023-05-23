<?hh // partial

namespace HH {

enum FactTypeFlags : int {
  // underscores avoid clashes PHP's (case-insensitive) keywords
  _ABSTRACT = 1;
  _FINAL = 2;
  // if this is set the flags are a union of features from all declarations
  MULTIPLE_DECLARATIONS = 4;
}

/**
 * Read a set of HH/PHP files and quickly extract the main features aka. "Facts"
 * about their contents. Compared to require'ing a source file this is much
 * faster and doesn't cause excessive memory consumption as compiled results are
 * not kept around.
 *
 * The intended use of this data is to gather info about a large number of
 * source files to perform queries about a codebase as a whole. For example this
 * can be used to make an autoloader or perform reflection queries of the form
 * "find all classes in a source tree which match pattern XYZ" etc.
 *
 * The useThreads argument specifies whether to use a number of worker threads
 * equal to the number of system CPU cores for parsing. This feature should only
 * be used as part of warm-up operations and not on a running system which may
 * have a high CPU load with new/in-flight requests.
 *
 * The allowHipHopSyntax argument enables HH syntax in all files parsed. By
 * default only files starting with `<?hh` may have HH syntax.
 *
 * Each pathList item will map to one result entry in the returned array. If
 * there were any errors when parsing a file the result entry for that file will
 * be null, otherwise a result will be of the form:
 *
 *   array(
 *     'sha1sum' => SHA1 Hash of source text,
 *     'types' => details of 'types' in the file (traits/classes/interfaces)
 *       array(
 *         array(
 *           'name' => string name of this 'type',
 *           'kindOf' => string =
 *              'class'|'interface'|'enum'|'trait'|'unknown'|'mixed',
 *           'baseTypes' => array(string) of base 'types' for this type,
 *           'flags' => integer with combination of flags from FactParseFlags,
 *           'requireImplements' => array(string) of interfaces users of this
 *             trait must implement (only present when kindOf = trait).
 *           'requireExtends' => array(string) of classes which users of this
 *             trait or interface must extend (only present when kindOf =
 *             trait | interface)
 *         ),
 *         array(
 *           'name' => string,
 *           'baseTypes' => array(string),
 *         ),
 *         ...
 *     ),
 *     'constants' => array(string) constants defined in this file,
 *     'functions' => array(string) functions defined in this file,
 *     'typeAliases' => array(string) type aliases defined in this file,
 *   )
 *
 * pathList items which are not absolute are joined with a non-null "root"
 * argument.
 */
<<__Native>>
function facts_parse(
  ?string $root,
  varray<string> $pathList,
  bool $allowHipHopSyntax,
  bool $useThreads,
): darray<string, ?darray<string, mixed>>;

/**
 * This should be bumped with every non-backwards compatible API change
 * 1 => first version
 * 2 => added $root argument to HH\facts_parse()
 * 3 => support requireExtends/requireImplements constraints
 */
function ext_factparse_version(): int {
  return (int)\phpversion("factparse");
}

} // namespace HH
