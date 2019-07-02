<?hh // strict

interface I1 {
  public const int MAX_ARGS = 5; // legal
  private const int MAX_ARGS_1 = 5;  // non_public_const_in_interface
  protected const int MAX_ARGS_2 = 5;  // non_public_const_in_interface
}
