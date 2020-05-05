<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

 /**
  * Object of this type is returned by `__FUNCTION_CREDENTIAL__`
  * attempting to construct an instance directly will throw an Error
  */
final class FunctionCredential {
  /**
   * Do NOT invoke the constructor.
   */
  private final function __construct() {}

  /**
   * The classname is equivalent to the magic `__CLASS__` constant.
   * The name is not a late bound classname.
   * It refers to `self::class`, not `static::class`.
   *
   * If `__FUNCTION_CREDENTIAL__` is used when not in the scope of a class,
   * the classname will be null.
   */
  public final function getClassName(): ?string;

  /**
   * The functionname is equivalent to the magic `__FUNCTION__` constant.
   */
  public final function getFunctionName(): string;

  /**
   * The absolute filename of the file.
   */
  public final function getFileName(): string;
}
