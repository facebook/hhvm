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
  * Object of this type is returned by __FUNCTION_CREDENTIAL__
  * but you almost certainly should NOT be attempting to construct this object
  */
final class FunctionCredential {
  private final function __construct() {}

  public final function getClassName(): ?string;

  public final function getFunctionName(): string;

  public final function getFilename(): string;

  public final function __getDebugInfo(): darray;
}
