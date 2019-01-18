<?hh // strict

final class C1 {
  <<__SoftLateInit>>
  private string $prop;

  <<__SoftLateInit>>
  private static string $prop2;
}

final class C2 {
  <<__SoftLateInit>>
  private string $prop = "1";
}
