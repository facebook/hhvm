<?hh

class ParentCls {
  protected int $withoutDefault;
  protected ?int $implicitDefaultInParent;
  protected int $withDefault = 13;
  protected int $defaultInChild;
  // This is a weird edge case I found. If you shadow a private member (totally
  // allowed), the logic we have can get confused: it can think the access to
  // the local private member is an access to the parent prop.
  private int $explicit;
  public function __construct() {
    $this->withoutDefault = 42;
    $this->explicit = 13;
    // Should be an error: we are accessing before initialization in the parent
    $this->defaultInChild;
    $this->defaultInChild = 42;
  }
}

class ChildCls extends ParentCls {

  protected int $inChildWithoutDefault;
  private ?int $implicitDefault;
  private int $explicit = 42;
  protected int $defaultInChild = 1;

  public function __construct(private int $promoted) {
    // Unconditionally initialized as part of calling _this_ constructor, so
    // should not error.
    $this->promoted;
    // This is fine as it has a default
    $this->withDefault;
    // This should only error once
    $this->unknownProp;
    // Not OK, as it might be uninitialized.
    $this->withoutDefault;
    // Ok, as this has an implicit default
    $this->implicitDefault;
    // Also ok, due to having an implicit default
    $this->implicitDefaultInParent;
    // Ok as this has an explicit default, and is _not_ the property set in
    // parent::__construct
    $this->explicit;
    // Ok, as this is a shadowed property given a default value
    $this->defaultInChild;
    parent::__construct();
    // We've called parent::__construct now, so this should be ok
    $this->withoutDefault;
    // This should _still_ be OK
    $this->withDefault;
    // This should be OK as it's a write
    $this->inChildWithoutDefault = 7;
  }

}
