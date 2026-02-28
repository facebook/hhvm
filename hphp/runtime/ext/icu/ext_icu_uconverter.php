<?hh

<<__NativeData>>
class UConverter implements IDisposable {
  /**
   * Create UConverter object
   *
   * @param string $destination_encoding -
   * @param string $source_encoding -
   *
   * @return  -
   */
  <<__Native>>
  public function __construct(string $destination_encoding = 'utf-8',
                              string $source_encoding = 'utf-8'): void;

  // TODO(4017519) PHP5 doesn't have this destructor, we have it to prevent a
  // nasty issue with destructing IntlUConverter.
  <<__Native>>
  public function __dispose(): void;

  /**
   * Convert string from one charset to another
   *
   * @param string $str -
   * @param bool $reverse -
   *
   * @return string -
   */
  <<__Native>>
  public function convert(string $str,
                          bool $reverse = false): mixed;

  /**
   * Default "from" callback function
   *
   * @param int $reason -
   * @param mixed $source -
   * @param string $codePoint -
   * @param int $error -
   *
   * @return mixed -
   */
  public function fromUCallback(int $reason,
                                mixed $source,
                                int $codePoint,
                                inout int $error): mixed {
    switch ($reason) {
      case self::REASON_UNASSIGNED:
      case self::REASON_ILLEGAL:
      case self::REASON_IRREGULAR:
        $error = U_ZERO_ERROR;
        return $this->getDestinationSubstChars();
      default: break;
    }
    return null;
  }

  /**
   * Get the aliases of the given name
   *
   * @param string $name -
   *
   * @return array -
   */
  <<__Native>>
  public static function getAliases(string $name): mixed;

  /**
   * Get the available canonical converter names
   *
   * @return array -
   */
  <<__Native>>
  public static function getAvailable(): varray;

  /**
   * Get the destination encoding
   *
   * @return string - (null on error)
   */
  <<__Native>>
  public function getDestinationEncoding(): ?string;

  /**
   * Get the destination converter type
   *
   * @return int -
   */
  <<__Native>>
  public function getDestinationType(): int;

  /**
   * Get last error code on the object
   *
   * @return int -
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message on the object
   *
   * @return string -
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the source encoding
   *
   * @return string - (null on error)
   */
  <<__Native>>
  public function getSourceEncoding(): ?string;

  /**
   * Get the source convertor type
   *
   * @return int -
   */
  <<__Native>>
  public function getSourceType(): int;

  /**
   * Get standard name
   *
   * @return string
   */
  <<__Native>>
  public static function getStandardName(string $name,
                                         string $standard): mixed;

  /**
   * Get mime name
   *
   * @return string
   */
  public static function getMimeName(string $name): mixed {
    return self::getStandardName($name, "MIME");
  }

  /**
   * Get standards associated to converter names
   *
   * @return array -
   */
  <<__Native>>
  public static function getStandards(): mixed;

  /**
   * Get substitution chars
   *
   * @return string -
   */
  <<__Native>>
  public function getDestinationSubstChars(): mixed;

  /**
   * Get substitution chars
   *
   * @return string -
   */
  <<__Native>>
  public function getSourceSubstChars(): mixed;

  /**
   * Get substitution chars
   *
   * @return string -
   */
  public function getSubstChars(): mixed {
    // Ambiguous, but mostly PHP compat
    // since PHP version lacks distinct setters
    return $this->getSourceSubstChars();
  }

  /**
   * Get string representation of the callback reason
   *
   * @param int $reason -
   *
   * @return string -
   */
  <<__Native>>
  public static function reasonText(int $reason): mixed;

  /**
   * Set the destination encoding
   *
   * @param string $encoding -
   *
   * @return bool -
   */
  <<__Native>>
  public function setDestinationEncoding(string $encoding): bool;

  /**
   * Set the source encoding
   *
   * @param string $encoding -
   *
   * @return void -
   */
  <<__Native>>
  public function setSourceEncoding(string $encoding): bool;

  /**
   * Set the substitution chars
   *
   * @param string $chars -
   *
   * @return void -
   */
  <<__Native>>
  public function setDestinationSubstChars(string $chars): bool;

  /**
   * Set the destination substitution chars
   *
   * @param string $chars -
   *
   * @return void -
   */
  <<__Native>>
  public function setSourceSubstChars(string $chars): bool;

  /**
   * Set the substitution chars
   *
   * @param string $chars -
   *
   * @return void -
   */
  public function setSubstChars(string $chars): bool {
    return $this->setSourceSubstChars($chars) &&
           $this->setDestinationSubstChars($chars);
  }

  /**
   * Default "to" callback function
   *
   * @param int $reason -
   * @param string $source -
   * @param string $codeUnits -
   * @param int $error -
   *
   * @return mixed -
   */
  public function toUCallback(int $reason,
                              ?string $source,
                              ?string $codeUnits,
                              inout int $error): mixed {
    switch ($reason) {
      case self::REASON_UNASSIGNED:
      case self::REASON_ILLEGAL:
      case self::REASON_IRREGULAR:
        $error = U_ZERO_ERROR;
        return $this->getSourceSubstChars();
      default: break;
    }
    return null;
  }

  /**
   * Convert string from one charset to another
   *
   * @param string $str -
   * @param string $toEncoding -
   * @param string $fromEncoding -
   * @param array $options -
   *
   * @return string -
   */
  public static function transcode(string $str,
                                   string $toEncoding,
                                   string $fromEncoding,
                                   ?darray $options = null): ?string {
    $cnv = new UConverter($toEncoding, $fromEncoding);
    if ((isset($options['from_subst']) &&
         !$cnv->setSourceSubstChars($options['from_subst'])) ||
        (isset($options['to_subst']) &&
         !$cnv->setDestinationSubstChars($options['to_subst']))) {
      return null;
    }
    return $cnv->convert($str);
  }
}
