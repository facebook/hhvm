<?hh // partial

/**
 * Guesses the encoding of an array of bytes in an
 * unknown encoding.
 *
 * http://icu-project.org/apiref/icu4c/ucsdet_8h.html
 */
<<__NativeData>>
class EncodingDetector {
  /**
   * Creates an encoding detector.
   */
  public function __construct(): void {}

  /**
   * Sets the input byte array whose encoding is to be guessed.
   *
   * @param string $texta - A byte array whose encoding is to be guessed.
   */
  <<__Native>>
  public function setText(string $text): void;

  /**
   * If the user provided an encoding in metadata
   * (like an HTTP or XML declaration),
   * this can be used as an additional hint to the detector.
   *
   * @param string $encoding - Possible encoding for the byte array obtained
   *                           from associated metadata
   */
  <<__Native>>
  public function setDeclaredEncoding(string $encoding): void;

  /**
   * Returns an EncodingMatch object containing the best guess
   * for the encoding of the byte array.
   *
   * @return object EncodingMatch
   */
  <<__Native>>
  public function detect(): EncodingMatch;

  /**
   * Returns an array of EncodingMatch objects containing all guesses
   * for the encoding of the byte array
   *
   * @return array<EncodingMatch> - Array of EncodingMatch objects for all
   *                                guesses of the encoding of the byte array
   */
  <<__Native>>
  public function detectAll(): varray<EncodingMatch>;
}

/**
  * Result of detecting the encoding of an array of bytes
  */
<<__NativeData>>
class EncodingMatch {
  /**
   * Internal only: Creates an encoding match.
   */
  private function __construct(): void {}

  /**
   * Checks if the encoding match succeeded.
   *
   * @return bool - true if the match succeeded, false otherwise
   */
  <<__Native>>
  public function isValid(): bool;

  /**
   * Gets the name of the detected encoding
   *
   * @return string - The name of the detected encoding
   */
  <<__Native>>
  public function getEncoding(): string;

  /**
   * Gets the confidence number of the encoding match
   *
   * @return int - Confidence number from 0 (no confidence) to 100
   *               (100 == complete confidence)
   */
  <<__Native>>
  public function getConfidence(): int;

  /**
   * Gets a rough guess at the language of the encoded bytes
   *
   * @return string - A rough guess at the language of the encoded bytes
   */
  <<__Native>>
  public function getLanguage(): string;

  /**
   * Gets the UTF-8 encoded version of the encoded byte array
   *
   * @return string - The result of converting the bytes to UTF-8
   *                  with the detected encoding
   */
  <<__Native>>
  public function getUTF8(): string;
}
