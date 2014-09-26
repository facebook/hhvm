<?

$MIN_POS = -100000;
$MAX_POS = 100000;


function generatePoints($n) {
  global $MIN_POS;
  global $MAX_POS;

  $points = array();
  for ($i = 0; $i < $n; $i++) {
    array_push($points, array(
      "x" => (float)rand($MIN_POS, $MAX_POS),
      "y" => (float)rand($MIN_POS, $MAX_POS)
    ));
  }
  array_unique($points, SORT_REGULAR);
  return $points;
}

function findMinXPos($points) {
  if (count($points) === 0) return null;

  $min_x_point = $points[0];
  for ($i = 1; $i < count($points); $i++) {
    $current_point = $points[$i];
    if ($min_x_point["x"] < $current_point["x"]) {
      $min_x_point = $current_point;
    }
  }

  return $min_x_point;
}

function side($a, $b, $c) {
  // Returns positive or negative depending on which side $c is when looking
  // from $a to $b.
  $determinant = ($b['x'] - $a['x']) * ($c['y'] - $a['y']) -
    ($b['y'] - $a['y']) * ($c['x'] - $a['x']);
  if ($determinant > 0) return 1;
  if ($determinant < 0) return -1;
  return 0;
}

function isInTriangle($p, $a, $b, $c) {
  list($p_x, $p_y) = array($p['x'], $p['y']);
  list($a_x, $a_y) = array($a['x'], $a['y']);
  list($b_x, $b_y) = array($b['x'], $b['y']);
  list($c_x, $c_y) = array($c['x'], $c['y']);
  $denom = ($b_y - $c_y) * ($a_x - $c_x) + ($c_x - $b_x) * ($a_y - $c_y);
  if ($denom > 0 || $denom < 0) {
    $alpha = (($b_y - $c_y) * ($p_x - $c_x) + ($c_x - $b_x) * ($p_y - $c_y)) /
      $denom;
    $beta = (($c_y - $a_y) * ($p_x - $c_x) + ($a_x - $c_x) * ($p_y - $c_y)) /
      $denom;
  } else {
    $alpha = 0.0;
    $beta = 0.0;
  }
  $gamma = 1.0 - $alpha - $beta;
  return $alpha > 0.0 && $beta > 0.0 && $gamma > 0.0;
}

function lineEquation($p1, $p2) {
  // y = mx + b
  // b = y - mx
  // mx - y + b = 0
  // ax + by + c = 0
  list($x1, $y1) = array($p1['x'], $p1['y']);
  list($x2, $y2) = array($p2['x'], $p2['y']);
  if ($x1 === $x2) {
    return array(
      "a" => 1.0,
      "b" => 0.0,
      "c" => -$x1
    );
  }
  $slope = ($y2 - $y1) / ($x2 - $x1);
  $b = $y1 - $slope * $x1;
  return array(
    "a" => $slope,
    "b" => -1.0,
    "c" => $b
  );
}

function distanceToLine($line, $p) {
  list($a, $b, $c) = array($line['a'], $line['b'], $line['c']);
  list($x_0, $y_0) = array($p['x'], $p['y']);
  return abs($a * $x_0 + $b * $y_0 + $c) / sqrt($a * $a + $b * $b);
}

function partitionSides($a, $b, $points) {
  return array(
    array_values(array_filter($points, function($p) use ($a, $b) {
      if ($p == $a || $p == $b)
        return false;
      return side($a, $b, $p) >= 0.0;
    })),
    array_values(array_filter($points, function($p) use ($a, $b) {
      if ($p == $a || $p == $b)
        return false;
      return side($a, $b, $p) < 0.0;
    }))
  );
}

function buildHullWithTriangles($a, $b, $points) {
  if (count($points) < 2) return $points;

  $hull = array();

  // Step 3: Find the point that is farthest from the dividing line.
  $line = lineEquation($a, $b);
  $farthest_dist = 0.0;
  $farthest_point = array_reduce($points,
    function($carry, $item) use ($line, &$farthest_dist) {
      $dist = distanceToLine($line, $item);
      if ($dist > $farthest_dist) {
        $farthest_dist = $dist;
        return $item;
      }
      return $carry;
    }, $points[0]);
  array_push($hull, $farthest_point);

  // Step 4: Filter out all points inside the triangle formed by the new point.
  $c = $farthest_point;
  $outsideTriangle = array_values(
    array_filter($points, function($p) use ($a, $b, $c) {
      if ($p == $a || $p == $b || $p == $c)
        return false;
      return !isInTriangle($p, $a, $b, $c);
    }));

  list($side1, $side2) = partitionSides($a, $c, $outsideTriangle);

  // Step 5: Using the two new lines, recursively create triangles to filter
  // points until there are no more left.
  $hull = array_merge(
    $hull,
    buildHullWithTriangles($a, $c, $side1),
    buildHullWithTriangles($b, $c, $side2)
  );
  return $hull;
}

function findConvexHull($points) {
  if (count($points) < 4) return $points;

  $hull = array();

  // Step 1: Find the two points with the min and max x positions.
  $min_point = array_reduce($points, function($carry, $item) {
    return ($item["x"] < $carry["x"]) ? $item : $carry;
  }, $points[0]);
  $max_point = array_reduce($points, function($carry, $item) {
    return ($item["x"] > $carry["x"]) ? $item : $carry;
  }, $points[0]);

  array_push($hull, $min_point, $max_point);

  // Step 2: Use the line between the two points to divide the remaining
  // points into two sets, one for each side of the line.
  list($side1, $side2) = partitionSides($min_point, $max_point, $points);

  $hull = array_merge(
    $hull,
    buildHullWithTriangles($min_point, $max_point, $side1),
    buildHullWithTriangles($min_point, $max_point, $side2)
  );

  return $hull;
}

function verifyConvexHull($actual, $expected) {
  return true;
}

$quickHull = function() {
  srand(42);
  $points = generatePoints(1000);
  $hull = findConvexHull($points);
};

$QuickHull = new BenchmarkSuite('QuickHull', [100000], array(
  new Benchmark('QuickHull', true, false, 4400, $quickHull)
));
