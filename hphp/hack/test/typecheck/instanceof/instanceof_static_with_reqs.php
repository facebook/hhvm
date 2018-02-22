<?hh

abstract class BaseField<T> {

  final public function __construct() {}

  final public static function fromType($type): this {
    $class_name = $type->getClassName();
    $field = new $class_name();
    hh_show($field);
    invariant(
      $field instanceof static,
      '%s is not a valid %s',
      $class_name,
      get_called_class(),
    );
    hh_show($field);

    if ($type instanceof HasFoo) {
      // This illustrates a limitation of the typechecker:
      // - intersection of this<BaseField> and INeedsFoo
      invariant(
        $field instanceof INeedsFoo,
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
  require extends Base;

  public function setFoo(T $arg): this;
}
