<?hh

namespace HH\ExperimentalParserUtils {

  function find_all_functions(darray $json)[]: dict<int, darray> {
    $funs = Map {};
    ffp_json_dfs(
      $json,
      false,
      ($j) ==> {
        if (\array_key_exists($j["kind"],
            keyset["lambda_expression", "anonymous_function", "methodish_declaration"])) {
          list($ln, $_) = find_boundary_token($j, false);
          invariant(!\array_key_exists($ln, $funs),
            "Files with multiple lambdas on the same line are not supported");

          $funs[$ln] = $j;
          return null; // forces traversal to continue to end
        }
        return null;
      },
      $_ ==> false,
    );
    return \HH\dict($funs);
  }

  function body_bounds(darray $function)[]: ((int, int), (int, int)) {
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

  function find_boundary_token(darray $body, bool $right)[]: ?(int, int) {
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
      ($json) ==>
        $json["kind"] === "attribute_specification"
        || $json["kind"] === "old_attribute_specification"
    );
  }

  function extract_name_from_node(darray $name_elem)[]: ?string {
    if ($name_elem["kind"] === "missing") {
      return null;
    }

    if ($name_elem["kind"] === "token") {
      return $name_elem["token"]["text"];
    }

    invariant($name_elem["kind"] === "qualified_name",
              "name kind must be missing, token, or qualified_name");
    $parts = vec[];
    foreach ($name_elem["qualified_name_parts"]["elements"] as $e) {
      if ($e["list_item"]["kind"] === "token") {
        $parts[] = $e["list_item"]["token"]["text"];
      } else {
        invariant($e["list_item"]["kind"] === "missing",
                  "qualified_name_parts may only be missing or tokens");
        $parts[] = '';
      }
    }
    return \implode('\\', $parts);
  }

  function find_method_names_in_concrete_derived_class(
    darray $class_decl,
    ?string $namespace
  )[]: vec<(string, string, string)> {
    if ($class_decl["classish_keyword"]["token"]["kind"] !== "class") {
      // ignore interfaces and traits
      return vec[];
    }
    if ($class_decl["classish_extends_list"]["kind"] !== "list") {
      // ignore non-derived classes
      return vec[];
    }

    invariant($class_decl["classish_name"]["kind"] === "token",
              "class name in declaration must be a token");
    $class_name = $namespace === null
      ? $class_decl["classish_name"]["token"]["text"]
      : $namespace . '\\' . $class_decl["classish_name"]["token"]["text"];

    invariant(\count($class_decl["classish_extends_list"]["elements"]) === 1,
              "class may only extend one parent");
    $parent_name = extract_name_from_node(
      $class_decl["classish_extends_list"]["elements"][0]["list_item"]["simple_type_specifier"]
    );
    invariant($parent_name !== null, "parent class may not have missing name");
    if ($namespace !== null && \substr($parent_name, 0, 1) !== '\\') {
      $parent_name = $namespace . '\\' . $parent_name;
    }

    $results = vec[];
    $class_elements = $class_decl["classish_body"]["classish_body_elements"]["elements"];
    foreach ($class_elements as $ce) {
      if ($ce["kind"] === "methodish_declaration") {
        $method_name = $ce["methodish_function_decl_header"]["function_name"]["token"]["text"];
        $results[] = tuple($parent_name, $class_name, $method_name);
      }
    }

    return $results;
  }

  function find_test_methods(darray $json)[]: vec<(string, string, string)> {
    $results = vec[];
    $namespace = null;

    $elements = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($elements as $e) {
      if ($e["kind"] === "namespace_declaration") {
        $old_namespace = $namespace;
        $header = $e["namespace_header"];
        $namespace = extract_name_from_node($header["namespace_name"]);

        // the `namespace A;` body kind is "namespace_empty_body"
        if ($e["namespace_body"]["kind"] === "namespace_body") {
          $namespace_elements = $e["namespace_body"]["namespace_declarations"]["elements"];
          foreach ($namespace_elements as $ne) {
            if ($ne["kind"] === "classish_declaration") {
              foreach (find_method_names_in_concrete_derived_class($ne, $namespace) as $r) {
                $results[] = $r;
              }
            }
          }

          // This namespace is ending, so resetting
          $namespace = $old_namespace;
        }
      } else if ($e["kind"] === "classish_declaration") {
        foreach (find_method_names_in_concrete_derived_class($e, $namespace) as $r) {
          $results[] = $r;
        }
      }
    }

    return $results;
  }

  // expects JSON of a file with a single type alias that is a shape
  function extract_type_of_only_shape_type_alias(darray $json)[]: dict<string, (string, bool)> {
    $elements = $json["parse_tree"]["script_declarations"]["elements"];
    invariant(\count($elements) === 3, "Supplied JSON must be parse tree of a single type alias.");
    invariant($elements[0]["kind"] === "markup_section",
      "Supplied JSON has unexpected form.");
    $type_alias = $elements[1];
    invariant($type_alias["kind"] === "alias_declaration",
      "Supplied JSON has unexpected form.");
    invariant($elements[2]["kind"] === "end_of_file",
      "Supplied JSON has unexpected form.");

    $shape_type = $type_alias["alias_type"];
    invariant($shape_type["kind"] === "shape_type_specifier",
      "Type alias does not point to a shape");

    $result = dict[];
    $fields = $shape_type["shape_type_fields"];
    if ($fields["kind"] === "missing") {
      return $result;
    }

    $ptext = $json["program_text"];
    foreach ($fields["elements"] as $field) {
      $field = $field["list_item"];
      $field_name = \trim($field["field_name"]["literal_expression"]["token"]["text"], "\"'");

      $ty = $field["field_type"];
      $left = find_boundary_token($ty, false);
      $right = find_boundary_token($ty, true);
      invariant($left !== null, "Failed to find left bound of field type");
      invariant($right !== null, "Failed to find right bound of field type");
      list($_, $l) = $left;
      list($_, $r) = $right;

      $type = \substr($ptext, $l, $r - $l);
      $optional = $field["field_question"]["kind"] !== "missing";
      $result[$field_name] = tuple($type, $optional);
    }

    return $result;
  }

  function find_enum_body(darray $json, string $name)[]: ?darray {
    $decls = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($decls as $d) {
      if ($d["kind"] === "enum_declaration") {
        $true_name = $d["enum_name"]["token"]["text"];
        if (\strcasecmp($true_name, $name) === 0) {
          return $d["enum_enumerators"];
        }
      }
    }
    return null;
  }

  function extract_enum_comments(darray $enumerators)[]: dict<string, vec<string>> {
    $result = dict[];
    if ($enumerators["kind"] === "missing") {
      return $result;
    }

    foreach ($enumerators["elements"] as $e) {
      $e_name = $e["enumerator_name"]["token"]["text"];
      $description = collect_comments($e);
      $result[$e_name] = $description;
    }

    return $result;
  }

  /**
   * Instead of doing a full recursion like the lambda extractor, this function
   * can do a shallow search of the tree to collect methods by name.
   * If there is a tie, use the line number
   */
  function find_method_parameters(darray $json, string $method_name, int $line_number)[]: darray {
    $candidates = vec[];
    $decls = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($decls as $d) {
      if ($d["kind"] === "classish_declaration") {
        $inner_decls = $d["classish_body"]["classish_body_elements"];
        if ($inner_decls["kind"] === "list") {
          foreach ($inner_decls["elements"] as $id) {
            if ($id["kind"] === "methodish_declaration") {
              $true_name = $id["methodish_function_decl_header"]["function_name"]["token"]["text"];
              if (\strcasecmp($true_name, $method_name) === 0) {
                $candidates[] = $id;
              }
            }
          }
        }
      }
    }

    $method = null;
    if (\count($candidates) === 1) {
      $method = $candidates[0];
    } else { // tiebreaker
      foreach ($candidates as $c) {
        $t = find_boundary_token($c, false);
        invariant($t !== null, "Failed to find function boundary");
        list($l, $_) = $t;
        if ($l === $line_number) {
          $method = $c;
          break;
        }
      }
    }

    invariant($method !== null, "Failed to find method in file");
    return $method["methodish_function_decl_header"]["function_parameter_list"];
  }

  function extract_parameter_comments(darray $params)[]: dict<string, vec<string>> {
    $result = dict[];
    if ($params["kind"] === "missing") {
      return $result;
    }

    foreach ($params["elements"] as $param) {
      $param_name_token = $param["list_item"]["parameter_name"]["token"];
      $param_name = \substr($param_name_token["text"], 1); // remove $

      $description = collect_comments($param);

      $result[$param_name] = $description;
    }

    return $result;
  }

  function find_class_method_shape_return_type(darray $class_body, string $name)[]: ?darray {
    $m = find_class_method($class_body, $name);
    if ($m === null) {
      return null;
    }
    $type = $m["methodish_function_decl_header"]["function_type"];
    $shape = extract_shape_from_type($type);
    return $shape;
  }

  function find_class_method(darray $class_body, string $name)[]: ?darray {
    $elements = $class_body["classish_body_elements"];
    if ($elements["kind"] === "missing") {
      return null;
    }

    foreach ($elements["elements"] as $e) {
      if ($e["kind"] === "methodish_declaration") {
        $true_name = $e["methodish_function_decl_header"]["function_name"]["token"]["text"];
        if (\strcasecmp($true_name, $name) === 0) {
          return $e;
        }
      }
    }
    return null;
  }

  function find_class_body(darray $json, string $name)[]: ?darray {
    $decls = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($decls as $d) {
      if ($d["kind"] === "classish_declaration") {
        $true_name = $d["classish_name"]["token"]["text"];
        if (\strcasecmp($true_name, $name) === 0) {
          return $d["classish_body"];
        }
      }
    }
    return null;
  }

  function find_class_shape_type_constant(darray $class_body, string $name)[]: ?darray {
    $elements = $class_body["classish_body_elements"];
    if ($elements["kind"] === "missing") {
      return null;
    }

    foreach ($elements["elements"] as $e) {
      if ($e["kind"] === "type_const_declaration") {
        $true_name = $e["type_const_name"]["token"]["text"];
        if (\strcasecmp($true_name, $name) === 0) {
          $shape = extract_shape_from_type($e["type_const_type_specifier"]);
          return $shape;
        }
      }
    }
    return null;
  }

  function find_single_shape_type_alias(darray $json, string $name)[]: ?(string, darray) {
    $decls = $json["parse_tree"]["script_declarations"]["elements"];
    foreach ($decls as $d) {
      if ($d["kind"] === "alias_declaration") {
        $true_name = $d["alias_name"]["token"]["text"];
        if (\strcasecmp($true_name, $name) === 0) {
          $type = $d["alias_type"];
          return tuple($true_name, extract_shape_from_type($type));
        }
      }
    }
    return null;
  }

  function extract_shape_from_type(darray $type)[]: darray {
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
      invariant(\count($ty_list["elements"]) === 1,
        "Only one argument to the Awaitable is allowed");
      $type = $ty_list["elements"][0]["list_item"];
      return extract_shape_from_type($type);
    } else {
      throw new \InvalidArgumentException("Type must be a shape type");
    }
  }

  function extract_shape_comments(darray $shape)[]: dict<string, vec<string>> {
    $result = dict[];
    $fields = $shape["shape_type_fields"];
    if ($fields["kind"] === "missing") {
      return $result;
    }

    // do not access into "list_item", because the comma that separates it from the
    // next is a sibling in the tree, but its description belongs to the same element
    foreach ($shape["shape_type_fields"]["elements"] as $field) {
      $field_name_token = $field["list_item"]["field_name"]["literal_expression"]["token"];
      $field_name = \trim($field_name_token["text"], "\"'");

      $description = collect_comments($field);

      $result[$field_name] = $description;
    }

    return $result;
  }

  function collect_comments(
    darray<arraykey, mixed> $shape_field,
  )[]: vec<string> {
    $trivia = vec[];
    $stack = vec[$shape_field];
    while ($stack) {
      $json = $stack[\count($stack) - 1];
      unset($stack[\count($stack) - 1]);
      if (!\HH\is_any_array($json)) {
        continue;
      }
      if (\array_key_exists("leading", $json)) {
        foreach ($json["leading"] as $c) {
          $trivia[] = $c;
        }
      }
      if (\array_key_exists("trailing", $json)) {
        foreach ($json["trailing"] as $c) {
          $trivia[] = $c;
        }
      }
      foreach (\array_reverse($json) as $v) {
        $stack[] = $v;
      }
    }

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
    (function (varray_or_darray)[_]: ?varray_or_darray) $predicate,
    (function (varray_or_darray)[_]: bool) $skip_node,
  )[ctx $predicate, ctx $skip_node]: ?varray_or_darray {
    if (!\HH\is_any_array($json)) {
      return null;
    }

    if (\array_key_exists("kind", $json)) {
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
      $json = \array_reverse($json);
    }
    foreach ($json as $v) {
      $b = ffp_json_dfs($v, $right, $predicate, $skip_node);
      if ($b !== null) {
        return $b;
      }
    }
  }
}
