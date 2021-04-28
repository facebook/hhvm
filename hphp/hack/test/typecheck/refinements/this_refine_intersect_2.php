<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ITypedJSModuleWithFallback {
  require extends TypedJSModule;

  public static function getFallbackTypedExports(): this::TExports;
}

abstract class TypedJSModule
{
  protected ?this::TExports $rawExports = null;
  abstract const type TExports as shape(...);

  final private function getFallbackExportsIfExist(): ?this::TExports {
    if (
      $this is ITypedJSModuleWithFallback
    ) {
      //hh_show_env();
      //hh_log_level("tyconst", 2);
      $x = $this::getFallbackTypedExports();
      //hh_show($x);
      return $x;
    } else {
      return null;
    }
  }
}
