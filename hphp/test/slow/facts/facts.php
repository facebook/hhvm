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
  $flags = Shapes::idx($filters, 'flags');
  if ($flags is Container<_>) {
    $flags = vec($flags);
    \sort(inout $flags);
    $filters_for_json['flags'] = $flags;
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
  bool $include_interface_require_extends = false,
): void {
  if ($filters is nonnull) {
    $subtypes_json = jsonify_arr(HH\Facts\transitive_subtypes($type, $filters, $include_interface_require_extends));
    $filters_json = jsonify_filters($filters);
    print "Transitive subtypes of $type with filters $filters_json: $subtypes_json\n";
  } else {
    $subtypes_json = jsonify_arr(HH\Facts\transitive_subtypes($type, $filters, $include_interface_require_extends));
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

function print_file_attrs(
  string $file,
): void {
  $attrs = dict[];
  foreach (HH\Facts\file_attributes($file) as $attr) {
    $attrs[$attr] = HH\Facts\file_attribute_parameters($file, $attr);
  }
  \ksort(inout $attrs);
  $attrs_json = \json_encode($attrs);
  print "Attributes of $file: $attrs_json\n";
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
  try {
    $method_tuples = HH\Facts\methods_with_attribute($attr);
  } catch (Exception $e) {
    print HH\Lib\Str\format(
      'Threw %s:"%s" trying to get methods with %s%s',
      get_class($e), $e->getMessage(), $attr, "\n");
    return;
  }
  $methods = vec[];
  foreach ($method_tuples as list($type, $method)) {
    $methods[] = "$type::$method";
  }
  \sort(inout $methods);
  $methods_json = \json_encode($methods);
  print "Methods decorated with $attr: $methods_json\n";
}

function print_attr_files(
  classname<\HH\FileAttribute> $attr,
): void {
  $files = HH\Facts\files_with_attribute($attr);
  \sort(inout $files);
  $files_json = \json_encode($files);
  print "Files decorated with $attr: $files_json\n";
}

function print_attr_files_and_args(
  classname<\HH\FileAttribute> $attr,
): void {
  $result = HH\Facts\files_and_attr_args_with_attribute($attr);
  \sort(inout $result);
  $result_json = \json_encode($result);
  print "Files and attr args with $attr: $result_json\n";
}

function print_attr_value_files(
  classname<\HH\FileAttribute> $attr,
  string $value,
): void {
  $files = HH\Facts\files_with_attribute_and_any_value($attr, $value);
  \sort(inout $files);
  $files_json = \json_encode($files);
  print "Files decorated with $attr and value $value: $files_json\n";
}
function print_path_modules(
  string $path,
): void {
  $modules = HH\Facts\path_to_modules($path);
  foreach ($modules as $mod) {
    print "$path has $mod module definition\n";
  }

  $module_membership = HH\Facts\path_to_module_membership($path);
  if ($module_membership is null) {
    print "$path has no module membership\n";
  } else {
    print "$path has $module_membership module membership\n";
  }
}

function print_path_package(
  string $path,
): void {
  $package = HH\Facts\path_to_package($path);
  if ($package is null) {
    print "$path has no package\n";
  } else {
    print "$path has $package package override\n";
  }
}

function print_num_symbols(
  string $path,
): void {
  $modules = HH\Facts\path_to_modules($path);
  $num_modules = \count($modules);
  print "$path has $num_modules modules\n";

  $module_membership = HH\Facts\path_to_module_membership($path);
  if ($module_membership is null) {
    print "$path has no module membership\n";
  } else {
    print "$path has $module_membership module membership\n";
  }

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

function print_type_file_path(
  string $className,
): void {
  $path = HH\Facts\type_to_path($className);
  print "$className has path: $path\n";
}

function print_function_file_path(
  string $function_name,
): void {
  $path = HH\Facts\function_to_path($function_name);
  print "$function_name has path: $path\n";
}

function print_type_alias_file_path(
  string $alias,
): void {
  $path = HH\Facts\type_or_type_alias_to_path($alias);
  print "alias $alias from type_or_type_alias_to_path has path: $path\n";

  $path = HH\Facts\type_alias_to_path($alias);
  print "alias $alias from type_alias_to_path has path: $path\n";
}

function print_constant_file_path(
  string $constant,
): void {
  $path = HH\Facts\constant_to_path($constant);
  print "$constant has path: $path\n";
}

function print_module_file_path(
  string $module,
): void {
  $path = HH\Facts\module_to_path($module);
  print "$module has path: $path\n";
}

function print_validation(): void {
  //These shouldnt throw
  HH\Facts\validate(vec["someClass1", "classWithDuplicateName", "classWithDuplicateName"]);
  HH\Facts\validate(vec["classWithDuplicateName"]);

  //This should throw
  try {
    HH\Facts\validate();
  }
  catch(UnexpectedValueException $e){
    print "\n$e \n";
    print "UnexpectedValueException thrown as expected\n";
  }

  //This should throw
  try {
    HH\Facts\validate(vec["someClass1", "someClass1"]);
  }
  catch(UnexpectedValueException $e){
    print "UnexpectedValueException thrown as expected\n";
  }
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
  print_subtypes(
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
    BaseClass::class,
    shape('kind' => keyset[HH\Facts\TypeKind::K_CLASS]),
  );
  print_transitive_subtypes(
    SuperClassOfFinalClassB::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_CLASS,
        HH\Facts\TypeKind::K_TRAIT,
        HH\Facts\TypeKind::K_INTERFACE,
      ],
    ),
    true
  );
  print_transitive_subtypes(
    SuperClassOfFinalClassB::class,
    shape(
      'kind' => keyset[
        HH\Facts\TypeKind::K_CLASS,
        HH\Facts\TypeKind::K_TRAIT,
        HH\Facts\TypeKind::K_INTERFACE,
      ],
    ),
    false
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
  print_subtypes(
    BaseClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    BaseClass::class,
    shape(
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
      'flags' => keyset[HH\Facts\TypeFlag::K_FINAL],
    ),
  );
  print_subtypes(
    BaseClass::class,
    shape(
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
      'flags' => keyset[
          HH\Facts\TypeFlag::K_ABSTRACT,
          HH\Facts\TypeFlag::K_FINAL
      ],
    ),
  );
  print_subtypes(
    BaseClass::class,
    shape(
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
      'flags' => keyset[HH\Facts\TypeFlag::K_EMPTY],
    ),
  );
  print_subtypes(
    BaseClass::class,
    shape(
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
      'flags' => keyset[
        HH\Facts\TypeFlag::K_ABSTRACT,
        HH\Facts\TypeFlag::K_EMPTY,
        HH\Facts\TypeFlag::K_FINAL
      ],
    ),
  );
  print_subtypes(
    BaseClass::class,
    shape(
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS],
      'flags' => keyset[],
    ),
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
  // `require class` trait constraints
  print_supertypes(
    TRequireClassFinalClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    TRequireClassFinalClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_supertypes(
    FinalClassUsesTRequireClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    FinalClassUsesTRequireClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_supertypes(
    TRequireClassFinalClassB::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
  );
  print_subtypes(
    FinalClassUsesTRequireClassB::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_EXTENDS]),
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
  print_supertypes(
    TRequireExtendsBaseClassAndRequireImplementsIBase::class,
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
  print_subtypes(
    IBase::class,
    shape(
      'kind' => keyset[HH\Facts\TypeKind::K_TRAIT],
      'derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS],
    ),
  );
  // `require class` trait constraints
  print_supertypes(
    TRequireClassFinalClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_subtypes(
    TRequireClassFinalClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_supertypes(
    FinalClassUsesTRequireClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_subtypes(
    FinalClassUsesTRequireClass::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_supertypes(
    TRequireClassFinalClassB::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );
  print_subtypes(
    FinalClassUsesTRequireClassB::class,
    shape('derive_kind' => keyset[HH\Facts\DeriveKind::K_REQUIRE_EXTENDS]),
  );

  print "\nGetting paths\n";
  print_type_file_path(AppleThenBanana::class);
  print_function_file_path("outOfClassFunction");
  print_type_alias_file_path("AnAlias");
  print_constant_file_path("SOME_CONSTANT");
  print_module_file_path("someModule");
  print "\nGetting attributes\n";

  print_type_attrs(AppleThenBanana::class);
  print_type_attrs(BananaThenApple::class);
  print_type_attrs(AppleThenCarrot::class);
  print_type_attrs(ClassWithTwoAttrs::class);
  print_type_alias_attrs(TypeAliasWithAttr::class);
  print_method_attrs(ClassWithMethodAttrs::class, 'methodWithNoArgAttr');
  print_method_attrs(ClassWithMethodAttrs::class, 'methodWithTwoArgAttr');

  print
    "\nThese should be empty, otherwise we're mixing types and type aliases ".
    "up again\n";
  print_type_alias_attrs(ClassWithTwoAttrs::class);
  print_type_attrs(TypeAliasWithAttr::class);

  print "\nGetting types with attribute\n";
  print_attr_types(NoArgAttr::class);
  print_attr_types(TwoArgAttr::class);
  print_attr_methods(NoArgMethodAttr::class);
  print_attr_methods(TwoArgMethodAttr::class);
  print_attr_methods(DontIndexThisMethodAttr::class);

  print "\nGetting type aliases with attribute\n";
  print_attr_type_aliases(TypeAliasAttr::class);

  // The reason these are in a weird order is because there was a bug
  // which made print_files_and_args and print_attr_value_files not work
  // unless they were called *after* print_attr_files.  So scramble the
  // order aound to fix this.  The fix for that bug is in symbol-map.cpp
  // in the same commit as this comment.
  print "\nGetting file attributes\n";
  print_attr_value_files  (TwoArgFileAttr::class, 'Hello');
  print_attr_files_and_args(OneArgFileAttr::class);
  print_attr_files(NoArgFileAttr::class);
  print_attr_files(OneArgFileAttr::class);
  print_attr_files(TwoArgFileAttr::class);
  print_attr_files_and_args(NoArgFileAttr::class);
  print_attr_files_and_args(TwoArgFileAttr::class);
  print_file_attrs('attribute-classes.inc');
  print "\nGetting modules from paths\n";
  print_path_modules('types-with-kinds.inc');
  print_path_modules('module-membership.inc');
  print "\nGetting package overrides from paths\n";
  print_path_package('package-membership.inc');
  print_path_package('types-with-kinds.inc');
  print_path_package('does-not-exist.inc');
  print "\nChecking nonexistent paths\n";
  print_num_symbols('this/path/does/not/exist.php');
  print_num_symbols('/this/path/does/not/exist.php');
  print_validation();
  print "Finished.\n";
}
