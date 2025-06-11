<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>

namespace HH {
  newctx SimpliHack as [write_props];
}

namespace HH\SimpliHack {

  /**
   * Return the contents of the file at the given path. The path given is
   * relative to the root of the .hhconfig file. For instance if the directory
   * structure is:
   * /a/b/.hhconfig
   * /a/b/c/d/file.txt
   *
   * Then the path to file.txt is c/d/file.txt
   *
   * Note that this function can only be called within a SimpliHack
   * expression.
   *
   * @param string $file - Relative path from root of the file to read
   */
  function file(string $file)[\HH\SimpliHack]: string;

  function constructor(classname<mixed> $class)[\HH\SimpliHack]: string;

  /**
   * Return information about the methods of a class.
   *
   * Note that this function can only be called within a SimpliHack
   * expression.
   *
   * @param classname<mixed> $class - The class to get methods information for
   */
  function methods(classname<mixed> $class)[\HH\SimpliHack]: string;

  /**
   * Return information about the static methods of a class.
   *
   * Note that this function can only be called within a SimpliHack
   * expression.
   *
   * @param classname<mixed> $class - The class to get static methods information for
   */
  function static_methods(classname<mixed> $class)[\HH\SimpliHack]: string;

  /**
   * Return information about the instance fields of a class.
   *
   * Note that this function can only be called within a SimpliHack
   * expression.
   *
   * @param classname<mixed> $class - The class to get instance fields information for
   */
  function fields(classname<mixed> $class)[\HH\SimpliHack]: string;

  /**
   * Return information about the static fields of a class.
   *
   * Note that this function can only be called within a SimpliHack
   * expression.
   *
   * @param classname<mixed> $class - The class to get static fields information for
   */
  function static_fields(classname<mixed> $class)[\HH\SimpliHack]: string;
}
