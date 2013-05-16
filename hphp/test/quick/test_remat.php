<?

class KarmaMemcache {
  private $keyScoreArr;
  private $specs;

  function getKeyScores() {
    return $this->keyScoreArr;
  }
  function __construct() {
    $this->specs = array(
      "login_bruteforce_protection_delta_vetted_datr:3600" =>
      // A spec
        array('on_block_lockout_time' => 0,
          'max_score' => 10000000,
          'time_to_decay_max_score' => 100)
      );
  }

  private static function decayScore($a, $b, $c, $d) { return 0; }
  /**
   * Takes raw data fetched from memcache, applies necessary decay
   * function.
   */

  private function getFetchedXXX() {
    return array(1 /* score */, 0 /*last_occurrence*/, 0
    /*last_blocked_time*/);
  }
  public function getInfo() {
    $time = time();
    $this->keyScoreArr = array();
    foreach ($this->specs as $key => $spec) {
      $fetched = $this->getFetchedXXX();
      list($score, $last_occurrence, $last_blocked_time) =
        array(
          idx($fetched, 0, 0),
          idx($fetched, 1, 0),
          idx($fetched, 2, 0)
        );
      // FBLogger('test_remat')->warn("score starts $score\n");

      if ($time - $last_blocked_time > $spec['on_block_lockout_time']) {
        if ($score > $spec['max_score']) {
          // Only possible if thresholds were reduced.
          $score = 0.5 * $spec['max_score'];
          // FBLogger('test_remat')->warn("score halved max case $score\n");
        } else {
          $score = self::decayScore(
            $spec['max_score'],
            $spec['time_to_decay_max_score'],
            $score,
            $last_occurrence
          );
          // FBLogger('test_remat')->warn("score decayed $score\n");
        }
        $this->decayedKarmaVectors[$key] = array(
          'score' => (double)$score,
          'last_occurrence' => (int)$last_occurrence,
          'last_blocked_time' => (int)$last_blocked_time
          );
        // FBLogger('test_remat')->warn("decayed decayed $score: " .
        //         print_r($this->decayedKarmaVectors[$key], true));
      } else {
        // In on-block-lockout-interval.
        // Don't add to $this->decayedKarmaVectors. That would update the
        // timestamp.
      }

      $this->keyScoreArr[$key] = (double)$score;
      if ($this) {
        var_dump((double)$score, $this->keyScoreArr[$key]);
      }
    }
  }
}

function main() {
  $kc = new KarmaMemcache();
  $kc->getInfo();
  var_dump($kc->getKeyScores());
}
main();
