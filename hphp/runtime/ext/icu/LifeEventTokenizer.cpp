/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/ext/icu/LifeEventTokenizer.h"
#include <atomic>
#include <vector>

using namespace U_ICU_NAMESPACE;

namespace HPHP {


// Rules for ICU's RuleBasedBreakIterator class.
const char* strRules = "\n\
!!chain;\n\
$CR           = [\\p{Word_Break = CR}];\n\
$LF           = [\\p{Word_Break = LF}];\n\
$Newline      = [\\p{Word_Break = Newline}];\n\
$Extend       = [\\p{Word_Break = Extend}];\n\
$Format       = [\\p{Word_Break = Format}];\n\
$Katakana     = [\\p{Word_Break = Katakana}];\n\
$ALetter      = [\\p{Word_Break = ALetter}];\n\
$MidNumLet    = [\\p{Word_Break = MidNumLet}];\n\
$MidLetter    = [\\p{Word_Break = MidLetter}];\n\
$MidNum       = [\\p{Word_Break = MidNum}];\n\
$Numeric      = [\\p{Word_Break = Numeric}];\n\
$ExtendNumLet = [\\p{Word_Break = ExtendNumLet}];\n\
$dictionary   = [:LineBreak = Complex_Context:];\n\
$Control      = [\\p{Grapheme_Cluster_Break = Control}];\n\
$ALetterPlus  = [$ALetter [$dictionary-$Extend-$Control]];\n\
$KatakanaEx     = $Katakana     ($Extend |  $Format)*;\n\
$ALetterEx      = $ALetterPlus  ($Extend |  $Format)*;\n\
$MidNumLetEx    = $MidNumLet    ($Extend |  $Format)*;\n\
$MidLetterEx    = $MidLetter    ($Extend |  $Format)*;\n\
$MidNumEx       = $MidNum       ($Extend |  $Format)*;\n\
$NumericEx      = $Numeric      ($Extend |  $Format)*;\n\
$ExtendNumLetEx = $ExtendNumLet ($Extend |  $Format)*;\n\
$Hiragana       = [\\p{script=Hiragana}];\n\
$Ideographic    = [\\p{Ideographic}];\n\
$HiraganaEx     = $Hiragana     ($Extend |  $Format)*;\n\
$IdeographicEx  = $Ideographic  ($Extend |  $Format)*;\n\
# FB custom rules\n\
# Email address\n\
$EmailAddress = [A-Za-z0-9_\\-\\.]+\\@[A-Za-z][A-Za-z0-9_]+\\.[a-z]+;\n\
# URL\n\
$Url = [htpfgopers]+\\:\\/\\/[a-z0-9]+(\\.[a-z0-9]+)+\
([a-zA-Z0-9]?[a-zA-Z0-9\\.\\/]+)?;\n\
# Emoticon\n\
$Emoticon = \\>?[Xx8\\:\\;\\=]\\-?\\'?[=\\/\\\\\\{\\}\\)\\(\\]\\[\\*DOoPp]+;\n\
# Heart\n\
$Heart = (\\<3|\u2665);\n\
# Exclamation\n\
$Exclamation = [\\!1]*\\![\\!1]*;\n\
# Date\n\
$Date = ([01]?[0-9]|[12][0-9][0-9][0-9])\\/[0-9][0-9]\\/\
([0-9][0-9]|[12][0-9][0-9][0-9]);\n\
# Time\n\
$Time = [012345]?[0-9]\\:[012345][0-9](\\ [APap][Mm])?;\n\
# Money\n\
$Money = \\$[0-9]+(\\.[0-9][0-9])?;\n\
# Acronym\n\
$Acronym = [A-Z]\\.([A-Z]\\.)+;\n\
!!forward;\n\
$CR $LF;\n\
[^$CR $LF $Newline]? ($Extend |  $Format)+;\n\
$NumericEx {100};\n\
$ALetterEx {200};\n\
$KatakanaEx {300};\n\
$HiraganaEx {300};\n\
$IdeographicEx {400};\n\
$ALetterEx $ALetterEx {200};\n\
$ALetterEx ($MidLetterEx | $MidNumLetEx) $ALetterEx {200};\n\
$NumericEx $NumericEx {100};\n\
$ALetterEx $NumericEx {200};\n\
$NumericEx $ALetterEx {200};\n\
$NumericEx ($MidNumEx | $MidNumLetEx) $NumericEx {100};\n\
$KatakanaEx  $KatakanaEx {300};\n\
$ALetterEx      $ExtendNumLetEx {200};\n\
$NumericEx      $ExtendNumLetEx {100};\n\
$KatakanaEx     $ExtendNumLetEx {300};\n\
$ExtendNumLetEx $ExtendNumLetEx {200};\n\
$ExtendNumLetEx $ALetterEx  {200};\n\
$ExtendNumLetEx $NumericEx  {100};\n\
$ExtendNumLetEx $KatakanaEx {300};\n\
# FB custom\n\
$EmailAddress {500};\n\
$Url {501};\n\
$Emoticon {502};\n\
$Heart {503};\n\
$Exclamation {504};\n\
$Date {505};\n\
$Money {506};\n\
$Time {507};\n\
$Acronym {508};\n\
!!reverse;\n\
$BackALetterEx     = ($Format | $Extend)* $ALetterPlus;\n\
$BackMidNumLetEx   = ($Format | $Extend)* $MidNumLet;\n\
$BackNumericEx     = ($Format | $Extend)* $Numeric;\n\
$BackMidNumEx      = ($Format | $Extend)* $MidNum;\n\
$BackMidLetterEx   = ($Format | $Extend)* $MidLetter;\n\
$BackKatakanaEx    = ($Format | $Extend)* $Katakana;\n\
$BackExtendNumLetEx= ($Format | $Extend)* $ExtendNumLet;\n\
$LF $CR;\n\
($Format | $Extend)*  [^$CR $LF $Newline]?;\n\
$BackALetterEx $BackALetterEx;\n\
$BackALetterEx ($BackMidLetterEx | $BackMidNumLetEx) $BackALetterEx;\n\
$BackNumericEx $BackNumericEx;\n\
$BackNumericEx $BackALetterEx;\n\
$BackALetterEx $BackNumericEx;\n\
$BackNumericEx ($BackMidNumEx | $BackMidNumLetEx) $BackNumericEx;\n\
$BackKatakanaEx $BackKatakanaEx;\n\
$BackExtendNumLetEx ($BackALetterEx | $BackNumericEx | $BackKatakanaEx \
| $BackExtendNumLetEx);\n\
($BackALetterEx | $BackNumericEx | $BackKatakanaEx) $BackExtendNumLetEx;\n\
!!safe_reverse;\n\
($Extend | $Format)+ .?;\n\
($MidLetter | $MidNumLet) $BackALetterEx;\n\
($MidNum | $MidNumLet) $BackNumericEx;\n\
$dictionary $dictionary;\n\
!!safe_forward;\n\
($Extend | $Format)+ .?;\n\
($MidLetterEx | $MidNumLetEx) $ALetterEx;\n\
($MidNumEx | $MidNumLetEx) $NumericEx;\n\
$dictionary $dictionary;\n\
";


// Master copy of the tokenizer object. Uses the rules above.

std::atomic<const BreakIterator*> kMaster(nullptr);

const BreakIterator* getMaster() {
  if (auto master = kMaster.load(std::memory_order_acquire)) {
    return master;
  }
  UParseError parseError;
  UErrorCode errorCode = U_ZERO_ERROR;
  const BreakIterator* bi
    = new icu::RuleBasedBreakIterator(icu::UnicodeString(strRules),
                                      parseError,
                                      errorCode);
  // Atomically swap in bi, but delete it if this this thread loses the
  // initialization race.
  static const BreakIterator* expectedNull = nullptr;
  if (!kMaster.compare_exchange_strong(expectedNull, bi,
                                       std::memory_order_acq_rel)) {
    delete bi;
  }
  return kMaster.load(std::memory_order_acquire);
}

void tokenizeString(
  std::vector<Token>& tokenVectorOut,
  const BreakIterator* ptrBreakIterator,
  const icu::UnicodeString& ustr) {

  if (strRules == NULL) return;
  // icu::RuleBasedBreakIterator is stateful -- it cannot be used by
  // multiple threads simultaneously without causing data corruption and
  // crashes.
  //
  // Cloning is thread-safe (assuming ptrBreakIterator is not being
  // used while this is called) and light-weight, and the clone will
  // never be used by more than one thread, so we clone before
  // using the break iterator.
  //
  // We downcast back to RuleBasedBreakIterator because clone()
  // always returns a icu::BreakIterator, which doesn't implement
  // getRuleStatus().

  icu::RuleBasedBreakIterator *iter =
    dynamic_cast<icu::RuleBasedBreakIterator *>(
      ptrBreakIterator->clone());

  boost::scoped_ptr<icu::RuleBasedBreakIterator> breakIterator(iter);
  breakIterator->setText(ustr);

  std::vector<Token> tokenVector;
  int32_t start = breakIterator->first();
  int end = breakIterator->next();
  while (end != icu::BreakIterator::DONE) {
    int32_t ruleStatus = breakIterator->getRuleStatus();
    icu::UnicodeString ustrToken(ustr, start, end - start);
    tokenVector.push_back(Token(ustrToken, ruleStatus));
    start = end;
    end = breakIterator->next();
  }
  tokenVector.swap(tokenVectorOut);
}

}
