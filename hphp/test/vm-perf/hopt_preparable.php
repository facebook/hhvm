<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.


// Constants related to Preparables

class PreparableState {
  // Preparer does not know about this Preparable yet
  const UNPREPARED = 1;

  // Preparable is blocked by other Preparable or pending fetch
  const BLOCKED = 2;

  // Preparable is scheduled for execution (it's in Preparer's runnable set)
  const SCHEDULED = 3;

  // Preparable is currently running (current stack includes its __doRound())
  const RUNNING = 4;

  // Preparable successfully finished
  const FINISHED = 5;

  // Preparable failed with exception
  const FAILED = 6;
}


final class SignalFetch {
  // fetch required since begin of last prepare() call
  private static $requiresFetch = false;

  // preparables that are waiting for memcache_dispatch()
  private static $preparables = array();

  // each running memcache_dispatch() stores list of preparables to be informed
  private static $stack = array();


  // signal that fetch is needed
  public static function signal() {
    self::$requiresFetch = true;
  }

// SKIPPING code from www

  // attribute required fetches to the given Preparable
  public static function attributeTo(Preparable $p) {
    if (!self::$requiresFetch) {
      return false;
    }

    self::$requiresFetch = false;
    self::$preparables[] = $p;
    return true;
  }

// SKIPPING code from www

  // make sure no non-attributed fetch is requested
  public static function assertNotSignaled() {
    if (self::$requiresFetch) {
      invariant_violation(
        'Encountered non-attributed fetch request at invalid location'
      );
    }
  }
}


/**
 * A simple way of using this is to have a Preparable object which has (at
 * least) one method: prepare($pass).  An example of this object might be:
 *
 * class GetSecondDegreeFriends extends Preparable {
 *   public function __construct($user) {
 *     $this->user = $user;
 *   }
 *
 *   public function prepare($pass) {
 *     switch ($pass) {
 *     case 0:
 *       user_multiget_reciprocal_friends((array)$this->user, true,
 *                                        $this->friends);
 *       return true; // falls through below
 *     case 1:
 *       user_multiget_reciprocal_friends($this->friends[$this->user]), true,
 *                                        $this->second_degree_friends);
 *     }
 *     return false;
 *   }
 * }
 *
 * Preparables may also be composed. When in the context of a Preparable,
 * you may add other preparables to the $preparer via:
 *
 * // The next pass of the current preparable will not be executed until the
 * // OtherPreparable is finished
 * $preparer->waitFor(new OtherPreparable());
 *
 * // The current preparable may advance, but will not complete (and hence
 * // anything depending upon it will not advance) until the OtherPreparable
 * // is complete. If thinking about the dependency graph as a tree, these
 * // Preparables become siblings, and the parent will not continue until
 * // each of its children have completed.
 * $preparer->runSibling(new OtherPreparable());
 *
 * Note: as a convention, any method that should never be overridden/called from
 * outside flib/core/cache is prefixed with double underscores.  These methods
 * are public, as they need to be called from the Preparer.  If you are not the
 * Preparer, do not call them.
 *
 * Also note that our private variables are prefixed with double underscores as
 * well because of a HPHP bug.  Any subclass that uses the same variable names
 * will affect these variables as well.
 */

abstract class Preparable {

  const MAX_SEQUENTIAL_PASSES = 1024;

  /*
    This value is assigned to __next when the Preparable returns
    false.
   */
  const ALL_PASSES_COMPLETE = -808;

  /* Return values of handleException() */
  const EXCEPTION_HANDLED = 314159;
  const EXCEPTION_NOT_HANDLED = 271828;

  private

    $wasImported = false,
    $sequentialPasses = 0,

    /*
     * The last global pass that this Preparable ran in.  This is
     * from Preparer::$currentGlobalPass.
     */
    $lastPass = null,

    /** Depth of this preparable in the preparable tree.  Used
     * to detect explosions. */
    $depth = 0,

    /** Object for tracking profiling data */
    $__preparableProfEntry,

    /** Unique identifier for this Preparable object */
    $__index,

    /** Next variable to be given to prepare() as $pass */
    $__next = 0,

    /** state */
    $__state = PreparableState::UNPREPARED,

    /** The instance of the Preparer that's preparing this */
    $__preparer,

    /** A list of dependents that can run concurrently */
    $__siblings = array(),

    /** A list of dependents on which we are waiting */
    $__children = array(),

    /** A (non-unique) list of preparables that are depending on us */
    $__ancestors = array(),

    /** A flag indicating dependency on memcache_dispatch() */
    $__needsFetch = false,

    /**
     * In the unlikely case that we throw an Exception, we take care to
     * hang on to it and rethrow if ever added again.
     */
    $__exception;

  protected
    /**
     * The class name reported in the preparable stack, defaults to
     * get_class($this).
     */
    $__overrideClassName;



  /**
   * Do one or more rounds of prepare()
   *
   * @return true if a dispatch is required
   */
  final public function __doRound() {

//echo "__doRound()\n";
    $toProfile = !!$this->__preparableProfEntry;
    if ($this->__exception) {
      // This Preparable resulted in an Exception some time in the past.
      // Alas, it was readded and we must throw again.  Sorry :(
      if ($toProfile) {
        $this->__preparableProfEntry->throwException($this->__exception);
      }
      throw $this->__exception;
    }

    if ($toProfile) {
      // Make sure we don't share loaders with unrelated preparable in
      // the same pass
      LoaderCoordinator::clearPendingLoaders();
      $this->__preparableProfEntry->startTimer('do_round');
    }

    SignalFetch::assertNotSignaled();

    $this->sequentialPasses = 0;
    $this->__state = PreparableState::RUNNING;

    do {

      ++$this->sequentialPasses;

// SKIPPING CODE IN www

      $this->__needsFetch = SignalFetch::attributeTo($this);
    } while ($this->__canMakeProgress());

    // nested prep() could have failed us
    if ($this->__state !== PreparableState::FAILED) {
      $this->__state = PreparableState::BLOCKED;
    }

    /*
      This assignment must come after prepare(), otherwise an inline
      prep() may have incremented the pass after it has been saved.

      See comment in Preparer::moveToRunnableSet() for more
      discussion.

      If no fetch is required then we want to make sure $this can run
      immediately after becoming unblocked rather than waiting for the
      next dispatch.  Setting to null here causes moveToRunnableSet()
      to put this on the runnable queue to run immediately rather than
      the incomplete queue which runs after a dispatch.
     */
    $this->lastPass = $this->__needsFetch ? Preparer::getGlobalPass() : null;

    if ($toProfile) {
      $this->__preparableProfEntry->stopTimer('do_round');
      if ($this->__needsFetch) {
        $this->__preparableProfEntry->startTimer('dispatch');
        memcache_dispatch();
        $this->__preparableProfEntry->stopTimer('dispatch');
      }
    }
  }

  final public function __canMakeProgress() {
    $this->depth++;
    if ($this->depth > 7) {
      return false;
    }
    return true;

    //$has_block =
    //  $this->__needsFetch ||
    //  $this->__children ||
    //  $this->__siblings && $this->__allPassesComplete();
    //return !$has_block && !$this->__hasException();
  }

}

class C extends Preparable {
  public function foo() {
    for ( $i = 0 ; $i < 10000000 ; $i++ ) {
      $this->__doRound();
    }
  }
}

echo "Starting\n";

$c = new C;
$c->foo();

echo "Done\n";
