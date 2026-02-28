<?hh
<<file:__EnableUnstableFeatures('readonly')>>

/** Used to represent the type of object returned by __FUNCTION_CREDENTIAL__
 */
<<__NativeData>>
final class FunctionCredential {
  public final function __construct() {
    trigger_error("Can't create a FunctionCredential directly", E_ERROR);
  }

  <<__Native>>
  public final readonly function getClassName()[]: ?string;

  <<__Native>>
  public final readonly function getFunctionName()[]: string;

  <<__Native>>
  public final readonly function getFilename()[]: string;

  <<__Native>>
  public final readonly function pack()[]: string;

  <<__Native>>
  public static function unpack(string $packed)[]: FunctionCredential;

  public final readonly function __debugInfo(): darray<string, ?string> {
    return dict[
      'class_name' => $this->getClassName(),
      'function_name' => $this->getFunctionName(),
      'file_name' => $this->getFilename(),
    ];
  }
}
