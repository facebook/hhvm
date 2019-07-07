<?hh // partial

abstract class BaseField<T> {

  final public function __construct() {}

  final public static function fromType($type): this {
    $class_name = $type->getClassName();
    $field = new $class_name();
    hh_show($field);
    invariant(
      $field is this,
      '%s is not a valid %s',
      $class_name,
      /* HH_IGNORE_ERROR[2049] */ static::class,
    );
    hh_show($field);

    if ($type is HasFoo) {
      // This illustrates a limitation of the typechecker:
      // - intersection of this<BaseField> and INeedsFoo
      invariant(
        $field is INeedsFoo<_>,
        'The context arg was provided, but the field doesn\'t support one',
      );
      hh_show($field);
      $field->setFoo($type->getFoo());
    }

    hh_show($field);
    return $field;
  }
}

interface INeedsFoo<T> {
  public function setFoo(T $arg): this;
}

interface HasFoo {}
