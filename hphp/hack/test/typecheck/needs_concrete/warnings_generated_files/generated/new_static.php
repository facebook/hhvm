<?hh
// Test Uninstantiable_class_via_static warning is suppressed for generated files

<<__ConsistentConstruct>>
abstract class GeneratedNewStatic {
  public static function create_instance(): void {
    // No warning expected because this file matches /generated/ pattern
    // This would normally trigger Uninstantiable_class_via_static warning
    $_ = new static();
  }
}
