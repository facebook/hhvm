<?hh

class WWWTest {}

class Parent2 {
  <<__TestsBypassVisibility>>
  private function priv(): void {}
}

class ChildShadow extends Parent2 {
  private function priv(): void {} // error: cannot shadow __TestsBypassVisibility
}

class ParentWithPrivate {
  private function helper(): void {}
}

class ChildWithBypass extends ParentWithPrivate {
  <<__TestsBypassVisibility>>
  protected function helper(): void {} // ok: parent private is not bypassable
}

class ParentWithPrivateStatic {
  private static function staticHelper(): void {}
}

class ChildWithBypassStatic extends ParentWithPrivateStatic {
  <<__TestsBypassVisibility>>
  protected static function staticHelper(): void {} // ok: parent private is not bypassable
}

class ParentWithPrivateProp {
  private int $prop = 0;
}

class ChildWithBypassProp extends ParentWithPrivateProp {
  <<__TestsBypassVisibility>>
  protected int $prop = 0; // ok: parent private is not bypassable
}

class ParentWithPrivateStaticProp {
  private static int $staticProp = 0;
}

class ChildWithBypassStaticProp extends ParentWithPrivateStaticProp {
  <<__TestsBypassVisibility>>
  protected static int $staticProp = 0; // ok: parent private is not bypassable
}
