<?hh

class ScriptParamsSchema {}

final class ScriptEmptyController {
  const type TParams = ScriptParamsSchema;

  const vec<classname<this>> MY_CONST = vec[];

  protected function f(): void {
    HH\Lib\Vec\map(
      self::MY_CONST,
      $classname ==> type_structure($classname, 'TParams')['classname'],
    );
  }
}
