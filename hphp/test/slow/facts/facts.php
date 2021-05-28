<?hh

/**
 * Return the relative path of an absolute path.
 *
 * Assumes that this directory is the root directory.
 */
function relative_path(string $path): string {
  if (strlen($path) === 0 || $path[0] !== '/') {
    return $path;
  }
  $root = __DIR__;
  $offset = strpos($path, $root);
  if ($offset < 0) {
    print "FAILURE: $path does not begin with $root\n";
  }
  return substr($path, $offset + strlen($root) + 1);
}

function jsonify_arr(Container<arraykey> $arr): string {
  $arr = vec($arr);
  \sort(inout $arr);
  return json_encode($arr);
}

function jsonify_filters(HH\Facts\DeriveFilters $filters): string {
  $filters_for_json = dict[];
  $kinds = Shapes::idx($filters, 'kind');
  if ($kinds is Container<_>) {
    $kinds = vec($kinds);
    \sort(inout $kinds);
    $filters_for_json['kind'] = $kinds;
  }
  $derive_kinds = Shapes::idx($filters, 'derive_kind');
  if ($derive_kinds is Container<_>) {
    $derive_kinds = vec($derive_kinds);
    \sort(inout $derive_kinds);
    $filters_for_json['derive_kind'] = $derive_kinds;
  }
  $attributes = Shapes::idx($filters, 'attributes');
  if ($attributes is Container<_>) {
    $attributes = vec($attributes);
    \sort(inout $attributes);
    $filters_for_json['attributes'] = $attributes;
  }

  return json_encode($filters_for_json);
}

function print_kind(classname<mixed> $type): void {
  $kind = HH\Facts\kind($type);
  if ($kind is null) {
    print "$type has no kind\n";
  } else {
    print "$type is $kind\n";
  }
}

function print_subtypes(
  classname<mixed> $type,
  ?HH\Facts\DeriveFilters $filters = null,
): void {
  if ($filters is nonnull) {
    $subtypes_json = jsonify_arr(HH\Facts\subtypes($type, $filters));
    $filters_json = jsonify_filters($filters);
    print "Subtypes of $type with filters $filters_json: $subtypes_json\n";
  } else {
    $subtypes_json = jsonify_arr(HH\Facts\subtypes($type));
    print "Subtypes of $type: $subtypes_json\n";
  }
}

function print_transitive_subtypes(
  classname<mixed> $type,
  ?HH\Facts\DeriveFilters $filters = null,
): void {
  if ($filters is nonnull) {
    $subtypes_json = jsonify_arr(HH\Facts\transitive_subtypes(
      $type,
      $filters,
    ));
    $filters_json = jsonify_filters($filters);
    print "Transitive subtypes of $type with filters $filters_json: ".
      "$subtypes_json\n";
  } else {
    $subtypes_json = jsonify_arr(HH\Facts\transitive_subtypes($type));
    print "Transitive subtypes of $type: $subtypes_json\n";
  }
}

function print_supertypes(
  classname<mixed> $type,
  ?HH\Facts\DeriveFilters $filters = null,
): void {
  if ($filters is nonnull) {
    $supertypes_json = jsonify_arr(HH\Facts\supertypes($type, $filters));
    $filters_json = jsonify_filters($filters);
    print "Supertypes of $type with filters $filters_json: $supertypes_json\n";
  } else {
    $supertypes_json = jsonify_arr(HH\Facts\supertypes($type));
    print "Supertypes of $type: $supertypes_json\n";
  }
}

function print_type_attrs(
  classname<mixed> $type,
): void {
  $attrs = dict[];
  foreach (HH\Facts\type_attributes($type) as $attr) {
    $attrs[$attr] = HH\Facts\type_attribute_parameters($type, $attr);
  }
  \ksort(inout $attrs);
  $attrs_json = \json_encode($attrs);
  print "Attributes of $type: $attrs_json\n";
}

function print_type_alias_attrs(
  string $type_alias,
): void {
  $attrs = dict[];
  foreach (HH\Facts\type_alias_attributes($type_alias) as $attr) {
    $attrs[$attr] = HH\Facts\type_alias_attribute_parameters(
      $type_alias,
      $attr,
    );
  }
  \ksort(inout $attrs);
  $attrs_json = \json_encode($attrs);
  print "Attributes of $type_alias: $attrs_json\n";
}

function print_method_attrs(
  classname<nonnull> $type,
  string $method,
): void {
  $attrs = dict[];
  foreach (HH\Facts\method_attributes($type, $method) as $attr) {
    $attrs[$attr] = HH\Facts\method_attribute_parameters($type, $method, $attr);
  }
  \ksort(inout $attrs);
  $attrs_json = \json_encode($attrs);
  print "Attributes of $type::$method: $attrs_json\n";
}

function print_attr_types(
  classname<\HH\ClassAttribute> $attr,
): void {
  $types = HH\Facts\types_with_attribute($attr);
  \sort(inout $types);
  $types_json = \json_encode($types);
  print "Types decorated with $attr: $types_json\n";
}

function print_attr_type_aliases(
  classname<\HH\ClassAttribute> $attr,
): void {
  $type_aliases = HH\Facts\type_aliases_with_attribute($attr);
  \sort(inout $type_aliases);
  $type_aliases_json = \json_encode($type_aliases);
  print "Type aliases decorated with $attr: $type_aliases_json\n";
}

function print_attr_methods(
  classname<\HH\MethodAttribute> $attr,
): void {
  $method_tuples = HH\Facts\methods_with_attribute($attr);
  $methods = vec[];
  foreach ($method_tuples as list($type, $method)) {
    $methods[] = "$type::$method";
  }
  \sort(inout $methods);
  $methods_json = \json_encode($methods);
  print "Methods decorated with $attr: $methods_json\n";
}

function print_num_symbols(
  string $path,
): void {
  $types = HH\Facts\path_to_types($path);
  $num_types = \count($types);
  print "$path has $num_types types\n";

  $functions = HH\Facts\path_to_functions($path);
  $num_functions = \count($functions);
  print "$path has $num_functions functions\n";

  $constants = HH\Facts\path_to_constants($path);
  $num_constants = \count($constants);
  print "$path has $num_constants constants\n";

  $type_aliases = HH\Facts\path_to_type_aliases($path);
  $num_type_aliases = \count($type_aliases);
  print "$path has $num_type_aliases type aliases\n";
}

<<__EntryPoint>>
function facts(): void {
  var_dump(HH\Facts\enabled());

  var_dump(HH\Facts\TypeKind::K_CLASS);
  var_dump(HH\Facts\TypeKind::K_ENUM);
  var_dump(HH\Facts\TypeKind::K_INTERFACE);
  var_dump(HH\Facts\TypeKind::K_TRAIT);

  print_kind(BaseClass::class);
  print_kind(DerivedClass::class);

  print_kind(C1::class);
  print_kind(E1::class);
  print_kind(I1::class);
  print_kind(T1::class);
  print_kind(Nonexistent::class);

  print_subtypes(BaseClass::class);
  print_supertypes(BaseClass::class);

  print_subtypes(
    IBase::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_CLASS]),
  );
  print_transitive_subtypes(
    IBase::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_CLASS]),
  );
  print_subtypes(
    IBase::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_TRAIT]),
  );
  print_transitive_subtypes(
    IBase::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_TRAIT]),
  );
  print_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_CLASS,
        HH\Facts\TypeKind::K_TRAIT,
      ],
    ),
  );
  print_transitive_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_CLASS,
        HH\Facts\TypeKind::K_TRAIT,
      ],
    ),
  );

  print_supertypes(DerivedClass::class);
  print_supertypes(
    DerivedClass::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_CLASS]),
  );
  print_supertypes(
    DerivedClass::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_TRAIT]),
  );
  print_supertypes(
    DerivedClass::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_INTERFACE]),
  );
  print_supertypes(
    DerivedClass::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_CLASS,
        HH\Facts\TypeKind::K_TRAIT,
      ],
    ),
  );
  print_supertypes(
    DerivedClass::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_INTERFACE,
        HH\Facts\TypeKind::K_TRAIT,
      ],
    ),
  );

  print "\nExcluding `require extends` relations\n";

  print_subtypes(
    IBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_transitive_subtypes(
    IBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    BaseClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_transitive_subtypes(
    BaseClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_supertypes(
    TRequireExtendsBaseClassAndRequireImplementsIBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_supertypes(
    TRequireImplementsAndImplementsIBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_INTERFACE],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
    ),
  );
  print_transitive_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_INTERFACE],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
    ),
  );

  print "\nExcluding `extends` relations\n";

  print_subtypes(
    IBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_subtypes(
    BaseClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_transitive_subtypes(
    IBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_supertypes(
    TRequireExtendsBaseClassAndRequireImplementsIBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_transitive_subtypes(
    BaseClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_supertypes(
    TRequireImplementsAndImplementsIBase::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_supertypes(
    SomeEnum::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_CLASS]),
  );
  print_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_INTERFACE],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS],
    ),
  );
  print_transitive_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_INTERFACE],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS],
    ),
  );
  print_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_TRAIT],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS],
    ),
  );
  print_transitive_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_TRAIT],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS],
    ),
  );

  print "\nFiltering by attribute\n";

  print_subtypes(
    BaseClassForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[0 => "banana"],
    )]),
  );
  print_transitive_subtypes(
    IBaseForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[0 => "banana"],
    )]),
  );
  print_subtypes(
    BaseClassForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[0 => 'apple'],
    )]),
  );
  print_transitive_subtypes(
    IBaseForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[0 => 'apple'],
    )]),
  );
  print_subtypes(
    BaseClassForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[1 => 'carrot'],
    )]),
  );
  print_transitive_subtypes(
    IBaseForAttributeFiltering::class,
    shape('attributes' => vec[shape(
      'name' => TwoArgAttr::class,
      'parameters' => dict[1 => 'carrot'],
    )]),
  );

  print "\nGetting attributes\n";

  print_type_attrs(AppleThenBanana::class);
  print_type_attrs(BananaThenApple::class);
  print_type_attrs(AppleThenCarrot::class);
  print_type_attrs(ClassWithTwoAttrs::class);
  print_type_alias_attrs(TypeAliasWithAttr::class);
  print_method_attrs(ClassWithMethodAttrs::class, 'methodWithNoArgAttr');
  print_method_attrs(ClassWithMethodAttrs::class, 'methodWithTwoArgAttr');

  print "\nGetting types with attribute\n";
  print_attr_types(NoArgAttr::class);
  print_attr_types(TwoArgAttr::class);
  print_attr_methods(NoArgMethodAttr::class);
  print_attr_methods(TwoArgMethodAttr::class);

  print "\nGetting type aliases with attribute\n";
  print_attr_type_aliases(TypeAliasAttr::class);

  print "\nChecking nonexistent paths\n";
  print_num_symbols('this/path/does/not/exist.php');
  print_num_symbols('/this/path/does/not/exist.php');

  print "\nGetting all types\n";
  $all_types = HH\Facts\all_types();
  \ksort(inout $all_types);
  foreach ($all_types as $type => $path) {
    $is_abstract = HH\Facts\is_abstract($type) ? 'true' : 'false';
    $is_final = HH\Facts\is_final($type) ? 'true' : 'false';
    print "$type => $path\n";
    print "  Abstract? $is_abstract\n";
    print "  Final? $is_final\n";
    $facts_path = relative_path(HH\Facts\type_to_path($type) as nonnull);
    if ($facts_path !== $path) {
      print "FAILURE: $facts_path !== $path\n";
    }
  }
  print "\nGetting all functions\n";
  $all_functions = HH\Facts\all_functions();
  \ksort(inout $all_functions);
  foreach ($all_functions as $function => $path) {
    print "$function => $path\n";
    $facts_path = relative_path(
      HH\Facts\function_to_path($function) as nonnull,
    );
    if ($facts_path !== $path) {
      print "FAILURE: $facts_path !== $path\n";
    }
  }
  print "\nGetting all constants\n";
  $all_constants = HH\Facts\all_constants();
  \ksort(inout $all_constants);
  foreach ($all_constants as $constant => $path) {
    print "$constant => $path\n";
    $facts_path = relative_path(
      HH\Facts\constant_to_path($constant) as nonnull,
    );
    if ($facts_path !== $path) {
      print "FAILURE: $facts_path !== $path\n";
    }
  }
  print "\nGetting all type aliases\n";
  $all_type_aliases = HH\Facts\all_type_aliases();
  \ksort(inout $all_type_aliases);
  foreach ($all_type_aliases as $type_alias => $path) {
    print "$type_alias => $path\n";
    $facts_path = relative_path(
      HH\Facts\type_alias_to_path($type_alias) as nonnull,
    );
    if ($facts_path !== $path) {
      print "FAILURE: $facts_path !== $path\n";
    }
  }

  print "Finished.\n";
}
