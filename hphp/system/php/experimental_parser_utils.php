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

  function find_single_shape_type_alias(array $json, string $name): ?(string, array) {
    $decls = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($decls as $d) {
      if ($d["kind"] === "alias_declaration") {
        $true_name = $d["alias_name"]["token"]["text"];
        if (strcasecmp($true_name, $name) === 0) {
          $type = $d["alias_type"];
          return tuple($true_name, extract_shape_from_type($type));
        }
      }
    }
    return null;
  }

  function extract_shape_from_type(array $type): array {
    if ($type["kind"] === "shape_type_specifier") {
      return $type;
    } else if ($type["kind"] === "nullable_type_specifier") { // ?shape
      return extract_shape_from_type($type["nullable_type"]);
    } else if ($type["kind"] === "generic_type_specifier") { // Awaitable<...>
      invariant($type["generic_class_type"]["token"]["text"] === "Awaitable",
        "Generic return type must be Awaitable<shape(...)>");
      $ty_list = $type["generic_argument_list"]["type_arguments_types"];
      invariant($ty_list["kind"] === "list",
        "Awaitable must have an argument");
      invariant(count($ty_list["elements"]) === 1,
        "Only one argument to the Awaitable is allowed");
      $type = $ty_list["elements"][0]["list_item"];
      return extract_shape_from_type($type);
    } else {
      throw new \InvalidArgumentException("Type must be a shape type");
    }
  }

  function extract_shape_comments(array $shape): dict<string, vec<string>> {
    $result = dict[];
    $fields = $shape["shape_type_fields"];
    if ($fields["kind"] === "missing") {
      return $result;
    }

    // do not access into "list_item", because the comma that separates it from the
    // next is a sibling in the tree, but its description belongs to the same element
    foreach ($shape["shape_type_fields"]["elements"] as $field) {
      $field_name_token = $field["list_item"]["field_name"]["literal_expression"]["token"];
      $field_name = trim($field_name_token["text"], "\"'");

      $description = collect_comments($field);

      $result[$field_name] = $description;
    }

    return $result;
  }

  function collect_comments(array $shape_field): vec<string> {
    $trivia = Vector {};
    ffp_json_dfs(
      $shape_field,
      false,
      ($j) ==> {
        if (array_key_exists("leading", $j)) {
          $trivia->addAll($j["leading"]);
          $trivia->addAll($j["trailing"]);

          return null; // forces traversal to continue to end
        }
        return null;
      }
    );

    $comments = vec[];
    foreach ($trivia as $c) {
      $k = $c["kind"];
      $t = $c["text"];

      if ($k === "single_line_comment" ||
          $k === "delimited_comment") {
        $comments[] = $t;
      }
    }
    return $comments;
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
