<?hh // strict

namespace NS_determined_type;

function main(?int $p1_nint, num $p2_num, ?num $p3_nnum, mixed $p4_mixed, array<?int> $p5_arynint): void {
// play with a nullable type

//  $r = $p1_nint << 2;		// actual type might be int, but this has not been determined

  if (is_int($p1_nint)) {
    $r = $p1_nint << 2;	// type determined to be int
    var_dump($r);
  }

  if (!is_null($p1_nint)) {
    $r = $p1_nint << 2;	// isn't null, so type determined to be int
    var_dump($r);
  }

  if (is_null($p1_nint)) {
  } else {
    $r = $p1_nint << 2;	// isn't null, so type determined to be int
    var_dump($r);
  }

// play with a num

//  $r = $p2_num << 2;		// actual type might be int, but this has not been determined

  if (is_int($p2_num)) {
    $r = $p2_num << 2;	// type determined to be int
    var_dump($r);
  }

  if (!is_float($p2_num)) {
//***  	$r = $p2_num << 2;	// isn't float, so must be int
//  	var_dump($r);
  }

  if (is_float($p2_num)) {
  } else {
//***  $r = $p2_num << 2;	// isn't float, so must be int
//  var_dump($r);
  }

// play with a nullable num

//  $r = $p3_nnum << 2;		// actual type might be int, but this has not been determined

  if (is_int($p3_nnum)) {
    $r = $p3_nnum << 2;	// type determined to be int
    var_dump($r);
  }

  if (!is_null($p3_nnum) && !is_float($p3_nnum)) {
//***  $r = $p3_nnum << 2;	// type should be determined to be int
//    var_dump($r);
  }

// play with a mixed

//  $r = $p4_mixed << 2;		// actual type might be int, but this has not been determined

  if (is_int($p4_mixed)) {
    $r = $p4_mixed << 2;	// type determined to be int
    var_dump($r);
  }

// play with an array of nullable type

//  $r = $p5_arynint[0] << 2;		// actual type might be int, but this has not been determined

  if ((count($p5_arynint) > 0) && is_int($p5_arynint[0])) {
//***  $r = $p5_arynint[0] << 2;	// type should be determined to be int
//     var_dump($r);
  }

  if ((count($p5_arynint) > 0) && !is_null($p5_arynint[0])) {
//***  $r = $p5_arynint[0] << 2;	// isn't null, so must be an int
//     var_dump($r);
  }

  if ((count($p5_arynint) > 0) && is_null($p5_arynint[0])) {
  } else {
//***  $r = $p5_arynint[0] << 2;	// isn't null, so must be an int
//     var_dump($r);
  }
}

//main(10, 10, 10, 10, array(10));
