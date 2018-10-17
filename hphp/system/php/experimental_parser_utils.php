<?hh

namespace HH\ExperimentalParserUtils {

  function find_single_function(array $json, int $line): ?array {
    return ffp_json_dfs(
      $json,
      false,
      ($j) ==> {
        if (array_key_exists($j["kind"],
            keyset["lambda_expression", "anonymous_function", "methodish_declaration"])) {
          list($ln, $_) = find_boundary_token($j, false);
          if ($ln === $line) {
            return $j;
          }
        }
        return null;
      }
    );
  }

  function find_all_functions(array $json): dict<int, array> {
    $funs = Map {};
    ffp_json_dfs(
      $json,
      false,
      ($j) ==> {
        if (array_key_exists($j["kind"],
            keyset["lambda_expression", "anonymous_function", "methodish_declaration"])) {
          list($ln, $_) = find_boundary_token($j, false);
          invariant(!array_key_exists($ln, $funs),
            "Files with multiple lambdas on the same line are not supported");

          $funs[$ln] = $j;
          return null; // forces traversal to continue to end
        }
        return null;
      }
    );
    return \HH\dict($funs);
  }

  function body_bounds(array $function): ((int, int), (int, int)) {
    if ($function["kind"] === "lambda_expression") {
      $body = $function["lambda_body"];
    } else if ($function["kind"] === "anonymous_function") {
      $body = $function["anonymous_body"];
    } else {
      invariant($function["kind"] === "methodish_declaration",
                "Unexpected function kind");
      $body = $function["methodish_function_body"];
    }

    $left = find_boundary_token($body, false);
    $right = find_boundary_token($body, true);
    invariant($left !== null, "Failed to find left bound of function");
    invariant($right !== null, "Failed to find right bound of function");
    return tuple($left, $right);
  }

  function find_boundary_token(array $body, bool $right): ?(int, int) {
    return ffp_json_dfs(
      $body,
      $right,
      ($json) ==> {
        if ($json["kind"] === "token") {
          $t = $json["token"];
          $offset = $t["offset"];
          if ($right) {
            $offset += $t["leading_width"] + $t["width"];
          }
          return tuple($t["line_number"], $offset);
        }

        return null;
      },
      // This is necessary, otherwise the line returned by find_boundary_token and
      // ReflectionFunction::getStartLine may not match
      ($json) ==> $json["kind"] === "attribute_specification"
    );
  }




  // Simple depth-first search, supports early return
  function ffp_json_dfs(
    mixed $json, // this can be any value type valid in JSON
    bool $right,
    (function (array): array) $predicate,
    (function (array): bool) $skip_node = (($_) ==> false),
  ) {
    if (!is_array($json)) {
      return null;
    }

    if (array_key_exists("kind", $json)) {
      // early return
      if ($skip_node($json)) {
        return null;
      }

      // base case
      $b = $predicate($json);
      if ($b !== null) {
        return $b;
      }
    }

    // recursive case
    if ($right) {
      $json = array_reverse($json);
    }
    foreach ($json as $v) {
      $b = ffp_json_dfs($v, $right, $predicate, $skip_node);
      if ($b !== null) {
        return $b;
      }
    }
  }
}
