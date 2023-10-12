<?hh

class MyClassName {
  const WHITE = "white";
  const string TRUE = "true";
  const Serializable<string> BLUE = "blue\u{0123}";
  public const string PublicConst = "this should fail";
  protected const string ProtectedConst = "this should fail";
  private const string PrivateConst = "this should fail";
}
