<?php

class RegexIterator extends FilterIterator
{
  /* Constants */

  const MATCH = 0;
  const GET_MATCH = 1;
  const ALL_MATCHES = 2;
  const SPLIT = 3;
  const REPLACE = 4;

  const USE_KEY = 1;
  const INVERT_MATCH = 2;


  /* Properties */


  /**
   * @var string The regular expression to match.
   */
  private $regex;

  /**
   * @var integer Operation mode, see RegexIterator::setMode() for a list of
   *              modes.
   */
  private $mode;

  /**
   * @var integer Special flags, see RegexIterator::setFlags() for a list of
   *              available flags.
   */
  private $flags;

  /**
   * @var integer The regular expression flags.
   */
  private $pregFlags;

  /**
   * @var mixed
   */
  private $key;

  /**
   * @var mixed
   */
  private $current;

  /**
   * @var string
   */
  public $replacement;


  /* Methods */


  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.construct.php )
   *
   * Create a new RegexIterator which filters an Iterator using a regular
   * expression.
   *
   * @param Iterator $iterator   The iterator to apply this regex filter to.
   * @param string   $regex      The regular expression to match.
   * @param integer  $mode       Operation mode, see RegexIterator::setMode()
   *                             for a list of modes.
   * @param integer  $flags      Special flags, see RegexIterator::setFlags()
   *                             for a list of available flags.
   * @param integer  $preg_flags The regular expression flags. These flags
   *                             depend on the operation mode parameter:
   *                         - RegexIterator::ALL_MATCHES: See preg_match_all().
   *                         - RegexIterator::GET_MATCH: See preg_match().
   *                         - RegexIterator::MATCH: See preg_match().
   *                         - RegexIterator::REPLACE: none.
   *                         - RegexIterator::SPLIT: See preg_split().
   */
  public function __construct(\Iterator $iterator, $regex, $mode = self::MATCH,
                              $flags = 0, $preg_flags = 0) {
    parent::__construct($iterator);

    $this->setMode($mode);

    $this->regex       = $regex;
    $this->flags       = $flags;
    $this->pregFlags   = $preg_flags;
    $this->replacement = '';
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.accept.php )
   *
   * Matches (string) RegexIterator::current() (or RegexIterator::key() if
   * the RegexIterator::USE_KEY flag is set) against the regular expression.
   *
   * @return boolean TRUE if a match, FALSE otherwise.
   */
  public function accept() {
    if (is_array(parent::current())) {
      return false;
    }

    $this->key     = parent::key();
    $this->current = parent::current();

    $matches = array();
    $useKey  = ($this->flags & self::USE_KEY);
    $subject = $useKey
      ? (string) $this->key
      : (string) $this->current;

    switch ($this->mode) {
      case self::MATCH:
        $ret = (preg_match($this->regex, $subject, $matches,
                           $this->pregFlags) > 0);
        break;
      case self::GET_MATCH:
        $this->current = array();

        $ret = (preg_match($this->regex, $subject, $this->current,
                           $this->pregFlags) > 0);
        break;
      case self::ALL_MATCHES:
        $this->current = array();

        $count = preg_match_all($this->regex, $subject, $this->current,
                       $this->pregFlags);

        $ret = $count > 0;
        break;
      case self::SPLIT:
        $this->current = preg_split($this->regex, $subject, null,
                                    $this->pregFlags);

        $ret = ($this->current && count($this->current) > 1);
        break;
      case self::REPLACE:
        $replace_count = 0;
        $result = preg_replace($this->regex, $this->replacement,
                               $subject, -1, $replace_count);

        if ($result === null || $replace_count == 0) {
          $ret = false;
          break;
        }

        if ($useKey) {
          $this->key = $result;

          $ret = true;
          break;
        }

        $this->current = $result;

        $ret = true;
        break;
      default:
        $ret = false;
        break;
    }
    if ($this->flags & self::INVERT_MATCH) {
      return !$ret;
    } else {
      return $ret;
    }
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/filteriterator.key.php )
   *
   * Get the current key.
   *
   * @return mixed
   */
  public function key() {
    return $this->key;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/filteriterator.current.php )
   *
   * Get the current element value.
   *
   * @return mixed
   */
  public function current() {
    return $this->current;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.getregex.php )
   *
   * Returns current regular expression.
   *
   * @return string
   */
  public function getRegex() {
    return $this->regex;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.getmode.php )
   *
   * Returns the operation mode, see RegexIterator::setMode() for the list
   * of operation modes.
   *
   * @return integer
   */
  public function getMode() {
    return $this->mode;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.getflags.php )
   *
   * Returns the flags, see RegexIterator::setFlags() for a list of available
   * flags.
   *
   * @return integer
   */
  public function getFlags() {
    return $this->flags;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.getpregflags.php )
   *
   * Returns the regular expression flags, see RegexIterator::__construct()
   * for the list of flags.
   *
   * @return integer
   */
  public function getPregFlags() {
    return $this->pregFlags;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.setmode.php )
   *
   * Sets the operation mode.
   *
   * @param integer $mode The operation mode.
   *                      The available modes are listed below. The actual
   *                      meanings of these modes are described in the
   *                      predefined constants.
   *                      - 0: RegexIterator::MATCH
   *                      - 1: RegexIterator::GET_MATCH
   *                      - 2: RegexIterator::ALL_MATCHES
   *                      - 3: RegexIterator::SPLIT
   *                      - 4: RegexIterator::REPLACE
   *
   * @throws InvalidArgumentException
   */
  public function setMode($mode) {
    $mode = (integer) $mode;

    if ($mode < self::MATCH || $mode > self::REPLACE) {
      throw new InvalidArgumentException(sprintf('Illegal mode %ld', $mode));
    }

    $this->mode = $mode;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.setflags.php )
   *
   * Sets the flags.
   *
   * @param integer $flags The flags to set, a bitmask of class constants.
   *                       The available flags are listed below. The actual
   *                       meanings of these flags are described in the
   *                       predefined constants.
   *                       - 1: RegexIterator::USE_KEY
   */
  public function setFlags($flags) {
    $this->flags = $flags;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/regexiterator.setpregflags.php )
   *
   * Sets the regular expression flags.
   *
   * @param integer $preg_flags The regular expression flags. See
   *                            RegexIterator::__construct() for an overview
   *                            of available flags.
   */
  public function setPregFlags($preg_flags) {
    $preg_flags = (integer) $preg_flags;

    $this->pregFlags = $preg_flags;
  }
}
