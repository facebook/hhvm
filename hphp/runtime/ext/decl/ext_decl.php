<?hh

/* Keep in sync with ext_decl.hhi */

namespace HH {

  type TypeExpr = string;

  // Use constants TYPE_CONSTRAINT_KIND_*
  // constraint_as, constraint_eq, constraint_super
  type TypeConstraintKind = string;
  const string TYPE_CONSTRAINT_KIND_AS = 'constraint_as';
  const string TYPE_CONSTRAINT_KIND_EQ = 'constraint_eq';
  const string TYPE_CONSTRAINT_KIND_SUPER = 'constraint_super';

  // Use constants VISBILITY_*
  // public, private, protected, internal
  type Visibility = string;
  const string VISIBILITY_PUBLIC = 'public';
  const string VISIBILITY_PRIVATE = 'private';
  const string VISIBILITY_PROTECTED = 'protected';
  const string VISIBILITY_INTERNAL = 'internal';

  // Use constants VARIANCE_*
  // covariant, contravariant, invariant
  type Variance = string;
  const string VARIANCE_COVARIANT = 'covariant';
  const string VARIANCE_CONTRAVARIANT = 'contravariant';
  const string VARIANCE_INVARIANT = 'invariant';

  // Use constants REIFIED_*
  // erased, soft_reified, reified
  type Reified = string;
  const string REIFIED_ERASED = 'erased';
  const string REIFIED_SOFT_REIFIED = 'soft_reified';
  const string REIFIED_REIFIED = 'reified';

  // Use constants CLASS_KIND_*
  // class, interface, trait, enum, enum_class
  type ClassKind = string;
  const string CLASS_KIND_CLASS = 'class';
  const string CLASS_KIND_INTERFACE = 'interface';
  const string CLASS_KIND_TRAIT = 'trait';
  const string CLASS_KIND_ENUM = 'enum';
  const string CLASS_KIND_ENUM_CLASS = 'enum_class';

  // Use constants TYPEDEF_VISIBILITY_*
  // transparent, opaque, opaque_module, case_type
  type TypedefVisibility = string;
  const string TYPEDEF_VISIBILITY_TRANSPARENT = 'transparent';
  const string TYPEDEF_VISIBILITY_OPAQUE = 'opaque';
  const string TYPEDEF_VISIBILITY_OPAQUE_MODULE = 'opaque_module';
  const string TYPEDEF_VISIBILITY_CASE_TYPE = 'case_type';

  // Use constants TYPE_STRUCTURE_KIND_*
  // primitive, function, shape, other, union, intersection, tuple
  type ExtDeclTypeStructureKind = string;
  const string TYPE_STRUCTURE_KIND_PRIMITIVE = "primitive";
  const string TYPE_STRUCTURE_KIND_FUNCTION = "function";
  const string TYPE_STRUCTURE_KIND_SHAPE = "shape";
  const string TYPE_STRUCTURE_KIND_OTHER = "other";
  const string TYPE_STRUCTURE_KIND_UNION = "union";
  const string TYPE_STRUCTURE_KIND_INTERSECTION = "intersection";
  const string TYPE_STRUCTURE_KIND_TUPLE = "tuple";

  /*
   * In all the following ExtDecl* shapes, all the boolean values
   * will only show up if their value is true. In a similar way all
   * the other optional fields default to null or empty string.
   *
   * The reason is perf: There are a lot of fields and the non-default
   * values are sparse. Adding a lot of fields to each shape in the
   * runtime would add a cost that we could otherwise avoid.
   */

  type ExtDeclAttribute = shape(
    'name' => string,
    ?'args' => vec<string>,
  );

  type ExtDeclTypeConstraint = shape(
    'kind' => TypeConstraintKind,
    'type' => TypeExpr,
  );

  type ExtDeclFileConst = shape(
    'name' => string,
    'type' => TypeExpr,
  );

  type ExtDeclModule = shape(
    'name' => string,
    'imports' => vec<string>,
    'exports' => vec<string>,
  );

  type ExtDeclEnumType = shape(
    'base' => TypeExpr,
    ?'constraint' => TypeExpr,
    ?'includes' => vec<TypeExpr>,
  );

  type ExtDeclTypeConst = shape(
    'name' => string,
    'kind' => TypeExpr,
    ?'is_ctx' => bool,
    ?'is_enforceable' => bool,
    ?'is_refiable' => bool,
  );

  type ExtDeclTypedef = shape(
    'name' => string,
    'type' => TypeExpr,
    'visibility' => TypedefVisibility,
    ?'module' => string,
    ?'tparams' => vec<ExtDeclTParam>,
    ?'as_constraint' => TypeExpr,
    ?'super_constraint' => TypeExpr,
    ?'is_ctx' => bool,
    ?'is_internal' => bool,
    ?'docs_url' => bool,
    ?'attributes' => vec<ExtDeclAttribute>,
  );

  type ExtDeclClassConst = shape(
    'name' => string,
    'type' => TypeExpr,
    ?'is_abstract' => bool,
  );

  // https://github.com/facebook/hhvm/issues/5594
  type ExtDeclTParam_ = mixed;
  type ExtDeclTParam = shape(
    'name' => string,
    'variance' => Variance,
    'reified' => Reified,
    ?'attributes' => vec<ExtDeclAttribute>,
    ?'tparams' => vec<ExtDeclTParam_>,
    ?'constraints' => vec<ExtDeclTypeConstraint>,
  );

  type ExtDeclProp = shape(
    'name' => string,
    'type' => TypeExpr,
    'visibility' => Visibility,
    ?'is_abstract' => bool,
    ?'is_const' => bool,
    ?'is_lateinit' => bool,
    ?'is_lsb' => bool,
    ?'is_needs_init' => bool,
    ?'is_php_std_lib' => bool,
    ?'is_readonly' => bool,
    ?'is_safe_global_variable' => bool,
    ?'is_no_auto_likes' => bool,
  );

  type ExtDeclMethodParam = shape(
    'name' => string,
    'type' => TypeExpr,
    ?'is_soft_type' => bool,
    ?'is_accept_disposable' => bool,
    ?'is_inout' => bool,
    ?'has_default' => bool,
    ?'is_readonly' => bool,
  );

  type ExtDeclSignature = shape(
    'return_type' => TypeExpr,
    ?'tparams' => vec<ExtDeclTParam>,
    ?'where_constraints' => vec<ExtDeclTypeConstraint>,
    ?'is_soft_return_type' => bool,
    ?'params' => vec<ExtDeclMethodParam>,
    ?'implicit_params' => string,
    ?'cross_package' => string,
    ?'is_return_disposable' => bool,
    ?'is_coroutine' => bool,
    ?'is_async' => bool,
    ?'is_generator' => bool,
    ?'is_instantiated_targs' => bool,
    ?'is_function_pointer' => bool,
    ?'is_returns_readonly' => bool,
    ?'is_readonly_this' => bool,
    ?'is_support_dynamic_type' => bool,
    ?'is_memoized' => bool,
    ?'is_variadic' => bool,
  );

  type ExtDeclMethod = shape(
    'name' => string,
    'visibility' => Visibility,
    'signature_type' => TypeExpr,
    ?'signature' => ExtDeclSignature,
    ?'attributes' => vec<ExtDeclAttribute>,
    ?'is_abstract' => bool,
    ?'is_final' => bool,
    ?'is_override' => bool,
    ?'is_dynamically_callable' => bool,
    ?'is_php_std_lib' => bool,
    ?'is_support_dynamic_type' => bool,
  );

  type ExtDeclFileFunc = shape(
    'name' => string,
    'signature_type' => TypeExpr,
    ?'module' => string,
    ?'is_internal' => bool,
    ?'is_php_std_lib' => bool,
    ?'is_support_dynamic_type' => bool,
    ?'is_no_auto_dynamic' => bool,
    ?'is_no_auto_likes' => bool,
    ?'signature' => ExtDeclSignature,
  );

  type ExtDeclClass = shape(
    'name' => string,
    'kind' => ClassKind,

    ?'module' => string,
    ?'docs_url' => string,

    ?'is_final' => bool,
    ?'is_abstract' => bool,
    ?'is_internal' => bool,
    ?'is_strict' => bool,
    ?'is_support_dynamic_type' => bool,

    ?'extends' => vec<TypeExpr>,
    ?'uses' => vec<TypeExpr>,
    ?'implements' => vec<TypeExpr>,
    ?'require_extends' => vec<TypeExpr>,
    ?'require_implements' => vec<TypeExpr>,
    ?'require_class' => vec<TypeExpr>,

    // XHP related
    ?'is_xhp' => bool,
    ?'has_xhp' => bool,
    ?'is_xhp_marked_empty' => bool,
    ?'xhp_attr_uses' => vec<TypeExpr>,

    // Complex types
    ?'attributes' => vec<ExtDeclAttribute>,
    ?'methods' => vec<ExtDeclMethod>,
    ?'static_methods' => vec<ExtDeclMethod>,
    ?'constructor' => ExtDeclMethod,
    ?'typeconsts' => vec<ExtDeclTypeConst>,
    ?'consts' => vec<ExtDeclClassConst>,
    ?'where_constraints' => vec<ExtDeclTypeConstraint>,
    ?'tparams' => vec<ExtDeclTParam>,
    ?'enum_type' => ExtDeclEnumType,
    ?'props' => vec<ExtDeclProp>,
    ?'static_props' => vec<ExtDeclProp>,
  );

  type ExtDeclFile = shape(
    ?'typedefs' => vec<ExtDeclTypedef>,
    ?'functions' => vec<ExtDeclFileFunc>,
    ?'constants' => vec<ExtDeclFileConst>,
    ?'file_attributes' => vec<ExtDeclAttribute>,
    ?'modules' => vec<ExtDeclModule>,
    ?'classes' => vec<ExtDeclClass>,
    ?'disable_xhp_element_mangling' => bool,
    ?'has_first_pass_parse_errors' => bool,
    ?'is_strict' => bool,
  );

  type ExtDeclTypeStructure_ = mixed;
  type ExtDeclTypeStructureSubType = shape(
    ?'name' => string,
    ?'optional' => bool,
    'type' => ExtDeclTypeStructure_,
  );

  type ExtDeclTypeStructure = shape(
    'type' => TypeExpr,
    'kind' => ExtDeclTypeStructureKind,
    ?'is_nullable' => bool,
    ?'subtypes' => vec<ExtDeclTypeStructureSubType>,
  );

  /** The FileDecls class parses a file and provides declaration information.
   */
  <<__NativeData>>
  final class FileDecls {

    // We want to be explicit about whether we are parsing text or file
    // The access to this class is via static methods.
    private function __construct()[] {}

    /*
     * Parse arbitrary text without any caching. For source code
     * parsing, please use the parsePath method instead.
     *
     * @param string $text - the contents of the file to parse
     * @return FileDecls - a queryable instance for the parsed data
     */
    <<__Native>>
    public static function parseText(string $text)[]: FileDecls;

    /*
     * Parse a source file. May use cached data.
     *
     * @param string $path - the relative path of the file to parse
     * @return FileDecls - a queryable instance for the parsed data
     */
    <<__Native>>
    public static function parsePath(string $path)[]: FileDecls;

    /*
     * Parse a type expression into a nested shape.
     *
     * @param string $type_expression - the type expression to parse
     * @return ExtDeclTypeStructure - a nested shape describing the type
     *                                or null if the input is bad.
     */
    <<__Native>>
    public static function parseTypeExpression(string $type_expression)[]: ?ExtDeclTypeStructure;

    /*
     * If there has been any error in parsing, the instance will throw
     * on query operations. This method checks the error state.
     *
     * @return string - the erroneous state or null if no errors
     */
    <<__Native>>
    public function getError()[]: ?string;

    /*
     * Checks the declaration for any class or typedef with the given name.
     *
     * @param string $name - the class or type name with or without the
     *                       global namespace prefix
     * @return bool - true if the class or typedef with that name exists
     */
    <<__Native>>
    public function hasType(string $name)[]: bool;

    /*
     * Query the content for all the information.
     *
     * @return ExtDeclFile - A non nullable whole file shape
     */
    <<__Native>>
    public function getFile()[]: ?ExtDeclFile;

    /*
     * Query the content for all classes.
     *
     * @return vec<ExtDeclClass> - Array of all classes in the content
     */
    <<__Native>>
    public function getClasses()[]: vec<ExtDeclClass>;

    /*
     * Query the content for a specific class.
     *
     * @param string $name - the class name with or without the global
     *                       namespace prefix
     * @return ?ExtDeclClass - The class shape or null if not found
     */
    <<__Native>>
    public function getClass(string $name)[]: ?ExtDeclClass;

    /*
     * Query the content for all the top level file attributes.
     *
     * @return vec<ExtDeclAttribute> - Array of the file attributes
     */
    <<__Native>>
    public function getFileAttributes()[]: vec<ExtDeclAttribute>;

    /*
     * Query the content for a specific top level file attribute.
     *
     * @param string $name - the attribute name with or without the global
     *                       namespace prefix
     * @return ?ExtDeclAttribute - The file attribute or null if not found
     */
    <<__Native>>
    public function getFileAttribute(string $name)[]: ?ExtDeclAttribute;

    /*
     * Query the content for all the top level consts.
     *
     * @return vec<ExtDeclFileConst> - Array of the consts
     */
    <<__Native>>
    public function getFileConsts()[]: vec<ExtDeclFileConst>;

    /*
     * Query the content a specific top level const.
     *
     * @param string $name - the const name
     * @return ?ExtDeclFileConst - The const or null if not found
     */
    <<__Native>>
    public function getFileConst(string $name)[]: ?ExtDeclFileConst;

    /*
     * Query the content for all the top level functions.
     *
     * @return vec<ExtDeclFileFunc> - Array of the functions
     */
    <<__Native>>
    public function getFileFuncs()[]: vec<ExtDeclFileFunc>;

    /*
     * Query the content for a specific top level function.
     *
     * @param string $name - the function name
     * @return ?ExtDeclFileFunc - The function or null if not found
     */
    <<__Native>>
    public function getFileFunc(string $name)[]: ?ExtDeclFileFunc;

    /*
     * Query the content for all the module definitions.
     *
     * @return vec<ExtDeclModule> - Array of the modules
     */
    <<__Native>>
    public function getFileModules()[]: vec<ExtDeclModule>;

    /*
     * Query the content for a specific module definition.
     *
     * @param string $name - the module name
     * @return ?ExtDeclModule - The module or null if not found
     */
    <<__Native>>
    public function getFileModule(string $name)[]: ?ExtDeclModule;

    /*
     * Query the content for all the top level type definitions.
     *
     * @return vec<ExtDeclTypedef> - Array of the type definitions
     */
    <<__Native>>
    public function getFileTypedefs()[]: vec<ExtDeclTypedef>;

    /*
     * Query the content for a specific top level type definition.
     *
     * @param string $name - the typedef name
     * @return ?ExtDeclTypedef - The type definition or null if not found
     */
    <<__Native>>
    public function getFileTypedef(string $name)[]: ?ExtDeclTypedef;

    /*
     * Query the content for all the methods of a specific class.
     *
     * @return vec<ExtDeclMethod> - Array of the methods
     */
    <<__Native>>
    public function getMethods(string $kls)[]: vec<ExtDeclMethod>;

    /*
     * Query the content for a specific method of a specific class.
     *
     * @param string $name - the method name
     * @return ?ExtDeclMethod - The method or null if not found
     */
    <<__Native>>
    public function getMethod(string $kls, string $name)[]: ?ExtDeclMethod;

    /*
     * Query the content for all the static methods of a specific class.
     *
     * @return vec<ExtDeclMethod> - Array of the static methods
     */
    <<__Native>>
    public function getStaticMethods(string $kls)[]: vec<ExtDeclMethod>;

    /*
     * Query the content for a specific static method of a specific class.
     *
     * @param string $name - the static method name
     * @return ?ExtDeclMethod - The static method or null if not found
     */
    <<__Native>>
    public function getStaticMethod(
      string $kls,
      string $name,
    )[]: ?ExtDeclMethod;

    /*
     * Query the content for all the consts of a specific class.
     *
     * @return vec<ExtDeclClassConst> - Array of the class constants
     */
    <<__Native>>
    public function getConsts(string $kls)[]: vec<ExtDeclClassConst>;

    /*
     * Query the content for a specific const of a specific class.
     *
     * @param string $name - the class const name
     * @return ?ExtDeclClassConst - The class constant or null if not found
     */
    <<__Native>>
    public function getConst(string $kls, string $name)[]: ?ExtDeclClassConst;

    /*
     * Query the content for all the type constants of a specific class.
     *
     * @return vec<ExtDeclTypeConst> - Array of the class type constants
     */
    <<__Native>>
    public function getTypeconsts(string $kls)[]: vec<ExtDeclTypeConst>;

    /*
     * Query the content for a specific type constant of a specific class.
     *
     * @param string $name - the class type const name
     * @return ?ExtDeclTypeConst - The class type constant or null if not found
     */
    <<__Native>>
    public function getTypeconst(
      string $kls,
      string $name,
    )[]: ?ExtDeclTypeConst;

    /*
     * Query the content for all the properties of a specific class.
     *
     * @return vec<ExtDeclProp> - Array of the class properties
     */
    <<__Native>>
    public function getProps(string $kls)[]: vec<ExtDeclProp>;

    /*
     * Query the content for a specific property of a specific class.
     *
     * @param string $name - the class property name without the dollar sign
     * @return ?ExtDeclProp - The class property or null if not found
     */
    <<__Native>>
    public function getProp(string $kls, string $name)[]: ?ExtDeclProp;

    /*
     * Query the content for all the static properties of a specific class.
     *
     * @return vec<ExtDeclProp> - Array of the class static properties
     */
    <<__Native>>
    public function getStaticProps(string $kls)[]: vec<ExtDeclProp>;

    /*
     * Query the content for a specific static property of a specific class.
     *
     * @param string $name - the class static property name with or without
     *                       the dollar sign
     * @return ?ExtDeclProp - The class static property or null if not found
     */
    <<__Native>>
    public function getStaticProp(string $kls, string $name)[]: ?ExtDeclProp;

    /*
     * Query the content for all the attributes of a specific class.
     *
     * @return vec<ExtDeclAttribute> - Array of the class attributes
     */
    <<__Native>>
    public function getAttributes(string $kls)[]: vec<ExtDeclAttribute>;

    /*
     * Query the content for a specific attribute of a specific class.
     *
     * @param string $name - the attribute name with or without the global
     *                       namespace prefix
     * @return ?ExtDeclAttribute - The class attribute or null if not found
     */
    <<__Native>>
    public function getAttribute(
      string $kls,
      string $name,
    )[]: ?ExtDeclAttribute;

    // Prevent cloning
    final public function __clone(): this {
      throw new \BadMethodCallException(
        'Trying to clone an uncloneable object of class FileDecls',
      );
    }

    public function __toString()[]: string {
      return __CLASS__;
    }
  }

}
