<?hh // strict
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


abstract class C {

  protected static dict<string, mixed> $data = dict[];

  enum E {
    case string name;
    case type T;

    :@I (name = 'I', type T = int);
  }

  // the localization of ?this:E:@TE:@T is calling into the option
  // smart constructor, which calls into subtyping routines.
  // During extends checks, this localization is done before TE is added
  // to the env tparams, which resulted in errors when extending C
  public static function set<TE as this:@E>(
    TE $entry,
    ?this:@E:@TE:@T $data = null): void {
    $name = static:@E::name($entry);
    static::$data[$name] = $data;
  }
}

class D extends C {
  enum E {
    :@S (name = 'S', type T = string);
  }
}
