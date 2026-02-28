<?hh

type SomeOtherType = string;
type SomeType = int;

abstract class Config {
  const type TParam = shape(
    'arg1' => keyset<SomeType>,
    'arg2' => keyset<SomeOtherType>,
     ...
  );
}

final class ConfigA extends Config {
  const type TParam = shape(
    'arg1' => keyset<SomeType>,
    'arg2' => keyset<SomeOtherType>,
    'arg3' => ?int,
  );
}

final class ConfigB extends Config {
  const type TParam = shape(
    'arg1' => keyset<SomeType>,
    'arg2' => keyset<SomeOtherType>,
    'arg4' => ?int,
  );
}

final class UseCase {
  const dict<string, Config::TParam> configDict = dict[];
}
