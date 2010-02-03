<?php
// Copyright 2004-2008 Facebook. All Rights Reserved.

/**
 * FBID Hashing Library
 *
 * Contains all the definitional information around the database IDs and the
 * way that user and object ids are hashed across them.
 *
 * @package core
 * @subpackage core.fbid
 */

function fbid_init() {
  /**
   * Database type definitions
   */
  define('DBTYPE_COLLEGE',     1);
  define('DBTYPE_TEST',        2);
  define('DBTYPE_GENERAL',     3);
  define('DBTYPE_HS',          4);
  define('DBTYPE_PLATFORM',    5);
  define('DBTYPE_ADMARKET',    6);
  define('DBTYPE_ANALYTICS',   7);
  define('DBTYPE_PLANALYTICS', 8);
  // NOTE: add a new type by modifying the above AND the below
  $GLOBALS['DBTYPE'] =
    array('COLLEGE'     => DBTYPE_COLLEGE,
          'TEST'        => DBTYPE_TEST,
          'GENERAL'     => DBTYPE_GENERAL,
          'HS'          => DBTYPE_HS,
          'PLATFORM'    => DBTYPE_PLATFORM,
          'ADMARKET'    => DBTYPE_ADMARKET,
          'ANALYTICS'   => DBTYPE_ANALYTICS,
          'PLANALYTICS' => DBTYPE_PLANALYTICS,
          );

  // 100K users per database
  $block_u = 100000;

  /**
   * Database layout configuration. This defines for each database type
   * the starting database id index, the number of logical databases in the
   * pool, and the block size of each database (which is the number of objects
   * allocated to that database).
   */
  $DBTYPE_CONFIG_ =
    array(DBTYPE_COLLEGE  =>
          array('min_dbid' => 1,
                'max_dbid' => 4990,
                'count'    => 4990,
                'block_u'  => $block_u,
                'min_uid'  => 1,
                'max_uid'  => 4990 * $block_u - 1,
                'block_o'  => 100000000),

          DBTYPE_TEST =>
          array('min_dbid' => 4991,
                'max_dbid' => 4999,
                'count'    => 9,
                'block_u'  => $block_u,
                'min_uid'  => 4990 * $block_u,  // NOTE: offset college_max
                'max_uid'  => 4999 * $block_u - 1,
                'block_o'  => 100000000),

          DBTYPE_GENERAL =>
          array('min_dbid' => 5000,
                'max_dbid' => 9999,
                'count'    => 5000,
                'block_u'  => $block_u,
                'min_uid'  => 5000 * $block_u,  // NOTE: offset general_min
                'max_uid'  => 9999 * $block_u - 1,
                'block_o'  => 100000000),

          DBTYPE_HS =>
          array('min_dbid' => 10000,
                'max_dbid' => 33004,            // count reserves up to 50000
                'count'    => 40001,            // moskov messup, 2005 =)
                'block_u'  => 30000,
                'min_uid'  => 9999 * $block_u,  // NOTE: offset general_max
                'block_o'  => 100000000),

          DBTYPE_PLATFORM =>
          array('min_dbid' => 50001,
                'max_dbid' => 51000,
                'count'    => 1000,
                'block_o'  => 1000000000),

          DBTYPE_ADMARKET =>
          array('min_dbid' => 51001,
                'max_dbid' => 51200,
                'count'    => 200,
                'block_o'  => 5000000000),

          DBTYPE_ANALYTICS =>
          array('min_dbid' => 52001,
                'max_dbid' => 52200,
                'count'    => 200,
                'block_o'  => 5000000000),

          DBTYPE_PLANALYTICS =>
          array('min_dbid' => 52201,
                'max_dbid' => 52400,
                'count'    => 200,
                'block_o'  => 100000000),
          );

  $GLOBALS['DBTYPE_USER_ORDER'] =
    array(DBTYPE_COLLEGE,
          DBTYPE_TEST,
          DBTYPE_GENERAL,
          DBTYPE_HS);

  $GLOBALS['DBTYPE_OBJECT_ORDER'] =
    array(DBTYPE_GENERAL,
          DBTYPE_COLLEGE,
          DBTYPE_TEST,
          DBTYPE_HS,
          DBTYPE_PLATFORM,
          DBTYPE_ADMARKET,
          DBTYPE_ANALYTICS);

  // Compute object space once
  $GLOBALS['first_object'] = 2200000000;

  // Offset general block object ids by the database count. I (mcslee) am
  // not sure why we do this, but it may be another moskov-hack circa 2005.
  $min_oid =
    $GLOBALS['first_object'] +
    $DBTYPE_CONFIG_[DBTYPE_GENERAL]['count'];
  foreach ($GLOBALS['DBTYPE_OBJECT_ORDER'] as $dbtype) {
    $config = &$DBTYPE_CONFIG_[$dbtype];
    $config['min_oid'] = $min_oid;
    $config['max_oid'] = $min_oid + ($config['count'] * $config['block_o']);

    // Next range starts at the end of this range, plus one! Guess what? The
    // plus one was another little unreversible error from back in the day.
    // It should have been that max_oid had a -1 in it, but this stuff is
    // no longer reversible now that the ids are all allocated.
    $min_oid = $config['max_oid'] + 1;
  }

  // Put the config into globals
  $GLOBALS['DBTYPE_CONFIG'] = $DBTYPE_CONFIG_;

  // Miscellaneous legacy stuff

  // this was basically choosen as just some nice number > 33004
  //    (the network_id of the last old-style launched hs)
  // the first new-style launched colleges and high schools have this network_id
  $GLOBALS['gemu_school_min_network_id'] = 50000;

  // corp networks are folded within the general dbs
  $GLOBALS['min_corp_id'] = 100000;
  $GLOBALS['max_corp_id'] = 500000;
}


/**
 * Checks if given id is in the fbid range (so not a user id)
 *
 * @param int $oid object id
 * @return bool Is this id in the object space
 */
function is_fbobj($oid) {
  global $first_object;
  return ($oid >= $first_object);
}

/**
 * Tells if some id looks like a Facebook user id
 * Needs to conform to is_fbobj, but don't do the function call for performance reasons.
 *
 * @param    int   $id
 * @return   bool  true if the id is in the user id range, which 0 is not
 *
 * @author   ccheever
 */
function is_user_id($id) {
  global $first_object;
  return ($id > 0 && $id < $first_object);
}

/**
 * Determines offset information for a given dbid. This provides the caller
 * with information about the starting object id for this dbtype range,
 * the number of dbs in this dbtype range, and the offset of this db into
 * its range.
 *
 * @param int $dbid
 * @return array keys start, num_dbs, db_offset
 */
function fbid_calculate_offset_info($dbid) {
  global $DBTYPE_CONFIG;
  foreach ($DBTYPE_CONFIG as $config) {
    if (($dbid >= $config['min_dbid']) &&
        ($dbid < $config['min_dbid'] + $config['count'])) {
      return array('start'     => $config['min_oid'],
                   'num_dbs'   => $config['count'],
                   'db_offset' => $dbid - $config['min_dbid']);
    }
  }
  debug_rlog('FBID: no offset into for dbid: '.$dbid);
}

/**
 * Converts a person or object id to the id of the db containing information
 * about the person or object. The object case (any id over 2.2B) simply
 * hashes the id into one of the available general buckets. The person case
 * is a piece-wise function. Currently, the first 4990 dbs are reserved for
 * college users and ids are seperated into 100000 space buckets. DBs 4491-4999
 * are reserved for test purposes. The next
 * 5000 dbs (5000 - 9999) represent the general buckets, and are divided
 * similar to the college ones in 100K chunks. HS dbs are similar, except
 * they start at db 10000 and user id 10K*100K, and the buckets are only
 * 30000 ids long.
 * <p>
 * example:<br>
 *   id_to_db(6) = 1<br>
 *   id_to_db(1000001003) = 10003<br>
 *   id_to_db(500001003) = 6003<br>
 *   id_to_db(2250001003) = 6003 (object)
 *
 * @param  int  $id          the id to be converted
 * @param  bool $log_errors  Should we generate error messages on failure.
 * @return int the id of the db housing information about this id
 */
function id_to_db($id, $log_errors=true) {
  if (!is_numeric($id)) {
    if ($log_errors) {
      debug_rlog("invalid call to ID_TO_DB $id");
    }
    return null;
  }

  if ($id == 0x7fffffff) {
    if ($log_errors) {
      debug_rlog(
        "0x7fffffff is an invalid object id!  ".
        "Did you put a 64-bit object id into a 32-bit mysql column?  ".
        "Please investigate.");
    }
    return null;
  }
  if ($id == 0x7fffffffffffffff) {
    if ($log_errors) {
      debug_rlog(
        "0x7fffffffffffffff is an invalid object id!  ".
        "Is some clown app overflowing a BIGINT column, or are we?");
    }
    return null;
  }

  global $DBTYPE_CONFIG;
  if ($id < $GLOBALS['first_object']) {
    // User id case, this is special-cased like whoa. If you take issue with
    // this, take it up with moskov. Don't try to clean it up, because these
    // ids are already allocated and you'll destroy everything.
    if ($id <= $DBTYPE_CONFIG[DBTYPE_COLLEGE]['max_uid']) {
      return (int)($id / $DBTYPE_CONFIG[DBTYPE_COLLEGE]['block_u']) + 1;
    } else if ($id <= $DBTYPE_CONFIG[DBTYPE_TEST]['max_uid']) {
      return (int)($id / $DBTYPE_CONFIG[DBTYPE_TEST]['block_u']) + 1;
    } else if ($id < $DBTYPE_CONFIG[DBTYPE_GENERAL]['max_uid']) {
      // GEMU is different. < instead of <=, I don't know why. Also, ids
      // here are hashed across the DB space.
      return
        $DBTYPE_CONFIG[DBTYPE_GENERAL]['min_dbid'] +
        (int)($id % $DBTYPE_CONFIG[DBTYPE_GENERAL]['count']);
    } else {
      $min_dbid = $DBTYPE_CONFIG[DBTYPE_HS]['min_dbid'];
      $base = ($min_dbid - 1) * $DBTYPE_CONFIG[DBTYPE_COLLEGE]['block_u'];
      $offset = (int)(($id - $base) / $DBTYPE_CONFIG[DBTYPE_HS]['block_u']);
      return $min_dbid + $offset;
    }
  } else {
    // Object id case, this one is nice and general-ish
    foreach ($GLOBALS['DBTYPE_OBJECT_ORDER'] as $dbtype) {
      $config = $DBTYPE_CONFIG[$dbtype];
      if (($id >= $config['min_oid']) && ($id <= $config['max_oid'])) {
        $offset = $id - $config['min_oid'];
        return $config['min_dbid'] + ($offset % $config['count']);
      }
    }
    if ($log_errors) {
      debug_rlog('FBID: Cannot determine db_id for '.$id);
    }
  }

  return null;
}

/**
 * Equivalent to calling id_to_db with $log_errors=false, but more readable.
 *
 * @param  int  $id          the id to be converted
 * @return int the id of the db housing information about this id (or null).
 */
function untrusted_id_to_db($id) {
  return id_to_db($id, false);
}

/**
 * Maps the xid to a db by converting the xid into an
 * int and then using id_to_db
 *
 * @param int $xid     Xid to get db for
 * @param int $db_type db type to use for hashing the xid.
 *                     Defaults to DBTYPE_GENERAL
 * @return int         db_id with in the specified db type
 */
function xid_to_db($xid, $db_type=DBTYPE_GENERAL) {
  if (!isset($GLOBALS['DBTYPE_CONFIG'][$db_type])) {
    return NULL;
  }

  // crc32 is weird in the sense that it will return a signed int in
  // some situations and a non negative one in other. to get around
  // this whole issue, we'll grab the positive part of the int
  $id = (crc32($xid) & 0x7fffffff);

  // Hash the id to a range within the specified db type
  return
    $GLOBALS['DBTYPE_CONFIG'][$db_type]['min_dbid'] +
    ($id % $GLOBALS['DBTYPE_CONFIG'][$db_type]['count']);
}


/**
 * Converts a single array of fbid into a nested array keyed on dbid.
 *
 * @param array
 * @return array
 */
function ids_to_dbmap($ids) {
  $map = array();
  foreach ($ids as $id) {
    $map[id_to_db($id)][] = $id;
  }
  return $map;
}

/**
 * Returns a random dbid from the specified dbtype pool, or pools
 *
 * @param mixed $db_type could either be on specific type as in most use cases, or multiple parameters each denoting a type
 * @return int
 * @author mcslee
 */
function get_rand_db($db_type) {
  if (func_num_args() > 1) {
    $args = func_get_args();
    $sizes = array();
    $total = 0;
    foreach ($args as $index => $db_pool_type) {
      if (!isset($GLOBALS['DBTYPE_CONFIG'][$db_pool_type])) {
        debug_rlog('FBID: invalid dbtype to get_rand_db: '.$db_pool_type);
        return null;
      }
      $total += $GLOBALS['DBTYPE_CONFIG'][$db_pool_type]['count'];
      $sizes[$db_pool_type] = $total;
    }
    $index = mt_rand(1, $total);
    foreach ($sizes as $pool => $size) {
      if ($index <= $size) {
        $db_type = $pool;
        break;
      }
    }
  }
  if (!isset($GLOBALS['DBTYPE_CONFIG'][$db_type])) {
    debug_rlog('FBID: invalid dbtype to get_rand_db: '.$db_type);
    return null;
  }
  $config = $GLOBALS['DBTYPE_CONFIG'][$db_type];
  $min = $config['min_dbid'];
  $max = $config['max_dbid'];
  for ($i = 0; $i < 10; ++$i) {
    $dbid = mt_rand($min, $max);
    if (!check_down($dbid)) {
      break; // exit the loop if this connection is good
    }
  }
  // On QA DB, only use 1 out of 100 object DBs
  if (is_qa_server()) {
    $dbid = 100 * (int)($dbid/100);
  }
  return $dbid;
}


/**
 * Returns db_type given a db id
 *
 * @param  int $db_id  Database ID
 * @return int         DB Type
 * @author peter
 */
function dbid_to_dbtype($db_id) {
  global $DBTYPE_CONFIG;

  foreach ($DBTYPE_CONFIG as $db_type => $db_config) {
    if ($db_id >= $db_config['min_dbid'] &&
        $db_id <= $db_config['max_dbid']) {
      return $db_type;
    }
  }
  return null;
}

fbid_init();

PERF_START

// 200 id_to_db() calls
for ($i = 0; $i < 10; $i++) {
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
  id_to_db(503938184);
}

PERF_END
