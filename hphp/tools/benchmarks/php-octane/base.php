<?
// Copyright 2013 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Ported to PHP from Google's Octane v2.0 benchmarking suite for JavaScript.

// Performance.now is used in latency benchmarks, the fallback is Date.now.
$performance = array();
$performance['now'] = function() {
  return microtime(true) * 1000.0;
};

// Simple framework for running the benchmark suites and
// computing a score based on the timing measurements.


// A benchmark has a name (string) and a function that will be run to
// do the performance measurement. The optional setup and tearDown
// arguments are functions that will be invoked before and after
// running the benchmark, but the running time of these functions will
// not be accounted for in the benchmark score.
class Benchmark {
  public $name;
  public $doWarmup;
  public $doDeterministic;
  public $deterministicIterations;
  public $run;
  public $Setup;
  public $TearDown;
  public $rmsResult;
  public $minIterations;

  function __construct($name, $doWarmup, $doDeterministic,
    $deterministicIterations, $run, $setup = null, $tearDown = null,
    $rmsResult = null, $minIterations = 32) {
    $this->name = $name;
    $this->doWarmup = $doWarmup;
    $this->doDeterministic = $doDeterministic;
    $this->deterministicIterations = $deterministicIterations;
    $this->run = $run;
    $this->Setup = is_null($setup) ? function() { } : $setup;
    $this->TearDown = is_null($tearDown) ? function() { } : $tearDown;
    $this->rmsResult = $rmsResult;
    $this->minIterations = $minIterations;
  }
}


// Benchmark results hold the benchmark and the measured time used to
// run the benchmark. The benchmark score is computed later once a
// full benchmark suite has run to completion. If latency is set to 0
// then there is no latency score for this benchmark.
class BenchmarkResult {
  public $benchmark;
  public $time;
  public $latency;

  function __construct($benchmark, $time, $latency) {
    $this->benchmark = $benchmark;
    $this->time = $time;
    $this->latency = $latency;
  }

  // Automatically convert results to numbers. Used by the geometric
  // mean computation.
  function valueOf() {
    return $this->time;
  }
}


// Suites of benchmarks consist of a name and the set of benchmarks in
// addition to the reference timing that the final score will be based
// on. This way, all scores are relative to a reference run and higher
// scores implies better performance.
class BenchmarkSuite {
  public $name;
  public $reference;
  public $benchmarks;

  // Keep track of all declared benchmark suites.
  public static $suites = [];

  public static $scores = [];

  // Scores are not comparable across versions. Bump the version if
  // you're making changes that will affect that scores, e.g. if you add
  // a new benchmark or change an existing one.
  public static $version = '9';

  // Defines global benchsuite running mode that overrides benchmark suite
  // behavior. Intended to be set by the benchmark driver. Undefined
  // values here allow a benchmark to define behaviour itself.
  public static $config = array(
    "doWarmup" => null,
    "doDeterministic" => null
  );

  function __construct($name, $reference, $benchmarks) {
    $this->name = $name;
    $this->reference = $reference;
    $this->benchmarks = $benchmarks;
    array_push(BenchmarkSuite::$suites, $this);
  }

  // To make the benchmark results predictable, we replace Math.random
  // with a 100% deterministic alternative.
  public static function ResetRNG() {
    $seed = 49734321;
    srand($seed);
  }

  // Runs all registered benchmark suites and optionally yields between
  // each individual benchmark to avoid running for too long in the
  // context of browsers. Once done, the final score is reported to the
  // runner.
  public static function RunSuites($runner, $skipBenchmarks = null) {
    $skipBenchmarks = is_null($skipBenchmarks) ? array() : $skipBenchmarks;
    $continuation = null;
    $length = count(BenchmarkSuite::$suites);
    BenchmarkSuite::$scores = array();
    $suites = BenchmarkSuite::$suites;
    $index = 0;
    $RunStep = function() use ($continuation, $length, $index,
      $runner, $skipBenchmarks) {
      while ($continuation != null || $index < $length) {
        if ($continuation != null) {
          $continuation = $continuation();
        } else {
          $suite = BenchmarkSuite::$suites[$index++];
          if (array_key_exists('NotifyStart', $runner))
            $runner['NotifyStart']($suite->name);
          if (array_search($suite->name, $skipBenchmarks) > -1) {
            $suite->NotifySkipped($runner);
          } else {
            $continuation = $suite->RunStep($runner);
          }
        }
      }

      // show final result
      if (array_key_exists('NotifyScore', $runner)) {
        $score = BenchmarkSuite::GeometricMean(BenchmarkSuite::$scores);
        $formatted = BenchmarkSuite::FormatScore(100 * $score);
        $runner['NotifyScore']($formatted);
      }
    };
    $RunStep();
  }

  // Counts the total number of registered benchmarks. Useful for
  // showing progress as a percentage.
  public static function CountBenchmarks() {
    $result = 0;
    $suites = BenchmarkSuite::$suites;
    for ($i = 0; $i < count($suites); $i++) {
      $result += count($suites[$i]->benchmarks);
    }
    return $result;
  }

  // Computes the geometric mean of a set of numbers.
  public static function GeometricMean($numbers) {
    $log = 0;
    for ($i = 0; $i < count($numbers); $i++) {
      $log += log($numbers[$i]);
    }
    if (count($numbers) == 0) {
      return NAN;
    }
    return exp($log / count($numbers));
  }

  // Computes the geometric mean of a set of throughput time measurements.
  public static function GeometricMeanTime($measurements) {
    $log = 0;
    for ($i = 0; $i < count($measurements); $i++) {
      $log += log($measurements[$i]->time);
    }
    return exp($log / count($measurements));
  }


  // Computes the geometric mean of a set of rms measurements.
  public static function GeometricMeanLatency($measurements) {
    $log = 0;
    $hasLatencyResult = false;
    for ($i = 0; $i < count($measurements); $i++) {
      if ($measurements[$i]->latency != 0) {
        $log += log($measurements[$i]->latency);
        $hasLatencyResult = true;
      }
    }
    if ($hasLatencyResult) {
      return exp($log / count($measurements));
    } else {
      return 0;
    }
  }

  // Converts a score value to a string with at least three significant
  // digits.
  public static function FormatScore($value) {
    if ($value > 100) {
      return (string)round($value, 0);
    } else {
      return (string)round($value, 3);
    }
  }

  // Notifies the runner that we're done running a single benchmark in
  // the benchmark suite. This can be useful to report progress.
  function NotifyStep($result) {
    array_push($this->results, $result);
    if (array_key_exists('NotifyStep', $this->runner))
      $this->runner['NotifyStep']($result->benchmark->name);
  }

  // Notifies the runner that we're done with running a suite and that
  // we have a result which can be reported to the user if needed.
  function NotifyResult() {
    $mean = BenchmarkSuite::GeometricMeanTime($this->results);
    $score = $this->reference[0] / $mean;
    array_push(BenchmarkSuite::$scores, $score);
    if ($this->runner['NotifyResult'] != null) {
      $formatted = BenchmarkSuite::FormatScore(100 * $score);
      $this->runner['NotifyResult']($this->name, $formatted);
    }
    if (count($this->reference) == 2) {
      $meanLatency = BenchmarkSuite::GeometricMeanLatency(
        $this->results);
      if ($meanLatency != 0) {
        $scoreLatency = $this->reference[1] / $meanLatency;
        array_push(BenchmarkSuite::$scores, $scoreLatency);
        if ($this->runner['NotifyResult']) {
          $formattedLatency = BenchmarkSuite::FormatScore(
            100 * $scoreLatency);
          $this->runner['NotifyResult']($this->name . "Latency",
            $formattedLatency);
        }
      }
    }
  }


  function NotifySkipped($runner) {
    array_push(BenchmarkSuite::$scores, 1); // push default reference score
    if (array_key_exists('NotifyResult', $runner)) {
      $runner['NotifyResult']($this->name, "Skipped");
    }
  }


  // Notifies the runner that running a benchmark resulted in an error.
  function NotifyError($error) {
    if (array_key_exists('NotifyError', $this->runner)) {
      $this->runner['NotifyError']($this->name, $error);
    }
    if (array_key_exists('NotifyStep', $this->runner)) {
      $this->runner['NotifyStep']($this->name);
    }
  }


  // Runs a single benchmark for at least a second and computes the
  // average time it takes to run a single iteration.
  function RunSingleBenchmark($benchmark, &$data) {
    $config = BenchmarkSuite::$config;
    $doWarmup = array_key_exists('doWarmup', $config) ?
      $config['doWarmup'] : $benchmark->doWarmup;
    $doDeterministic = array_key_exists('doDeterministic', $config) ?
      $config['doDeterministic'] : $benchmark->doDeterministic;

    $Measure = function(&$data) use ($doDeterministic, $benchmark) {
      global $performance;

      $elapsed = 0;
      $start = $performance['now']();

      // Run either for 1 second or for the number of iterations
      // specified by minIterations, depending on the config flag
      // doDeterministic.
      $i = 0;
      while (
        ($doDeterministic && $i < $benchmark->deterministicIterations)
        || (!$doDeterministic && $elapsed < 1000)) {
        $run = $benchmark->run;
        $run();
        $elapsed = $performance['now']() - $start;
        $i++;
      }
      if ($data != null) {
        $data['runs'] += $i;
        $data['elapsed'] += $elapsed;
      }
    };

    // Sets up data in order to skip or not the warmup phase.
    if (!$doWarmup && $data == null) {
      $data = array('runs' => 0, 'elapsed' => 0 );
    }

    if ($data == null) {
      $Measure(null);
      return array('runs' => 0, 'elapsed' => 0);
    } else {
      $Measure($data);
      // If we've run too few iterations, we continue for another second.
      if ($data['runs'] < $benchmark->minIterations)
        return $data;
      $usec = ($data['elapsed'] * 1000) / $data['runs'];
      $rms = 0;
      if ($benchmark->rmsResult != null) {
        $rmsResult = $benchmark->rmsResult;
        $rms = $rmsResult();
      }
      $this->NotifyStep(new BenchmarkResult($benchmark, $usec, $rms));
      return null;
    }
  }


  // This function starts running a suite, but stops between each
  // individual benchmark in the suite and returns a continuation
  // function which can be invoked to run the next benchmark. Once the
  // last benchmark has been executed, null is returned.
  function RunStep($runner) {
    BenchmarkSuite::ResetRNG();
    $this->results = array();
    $this->runner = $runner;
    $length = count($this->benchmarks);
    $index = 0;
    $suite = $this;
    $data = null;

    $RunNextSetup = null;
    $RunNextBenchmark = null;
    $RunNextTearDown = null;


    // Run the setup, the actual benchmark, and the tear down in three
    // separate steps to allow the framework to yield between any of the
    // steps.

    $RunNextSetup = function() use (
      $suite, &$index, $length, &$RunNextBenchmark) {
      if ($index < $length) {
        try {
          $setup = $suite->benchmarks[$index]->Setup;
          $setup();
        } catch (Exception $e) {
          $suite->NotifyError($e);
          return null;
        }
        return $RunNextBenchmark;
      }
      $suite->NotifyResult();
      return null;
    };

    $RunNextBenchmark = function() use (
      $suite, &$index, &$data, &$RunNextTearDown, &$RunNextBenchmark) {
      try {
        $data = $suite->RunSingleBenchmark(
          $suite->benchmarks[$index], $data);
      } catch (Exception $e) {
        $suite->NotifyError($e);
        return null;
      }
      // If data is null, we're done with this benchmark.
      return is_null($data) ? $RunNextTearDown : $RunNextBenchmark();
    };

    $RunNextTearDown = function() use ($suite, &$index, &$RunNextSetup) {
      try {
        $tearDown = $suite->benchmarks[$index++]->TearDown;
        $tearDown();
      } catch (Exception $e) {
        $suite->NotifyError($e);
        return null;
      }
      return $RunNextSetup;
    };

    // Start out running the setup.
    return $RunNextSetup();
  }
}

// Override the alert function to throw an exception instead.
$alert = function($s) {
  throw new Exception("Alert called with argument: $s");
};


?>
