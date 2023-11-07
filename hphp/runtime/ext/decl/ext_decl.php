<?hh

type TypeName = string;

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

type ExtDeclAttribute= shape(
  'name' => string,
  ?'args' => vec<string>,
);

type ExtDeclTypeConstraint = shape(
  'kind' => TypeConstraintKind,
  'type' => TypeName,
);

type ExtDeclFileConst = shape(
  'name' => string,
  'type' => TypeName,
);

type ExtDeclModule = shape(
  'name' => string,
  'imports' => vec<string>,
  'exports' => vec<string>,
);

type ExtDeclEnumType = shape(
  'base' => TypeName,
  ?'constraint' => TypeName,
  ?'includes' => vec<TypeName>
);

type ExtDeclTypeConst = shape(
  'name' => string,
  'kind' => TypeName,
  ?'is_ctx' => bool,
  ?'is_enforceable' => bool,
  ?'is_refiable' => bool,
);

type ExtDeclTypedef = shape(
  'name' => string,
  'type' => TypeName,
  'visibility' => TypedefVisibility,
  ?'module' => string,
  ?'tparams' => vec<ExtDeclTParam>,
  ?'as_constraint' => TypeName,
  ?'super_constraint' => TypeName,
  ?'is_ctx' => bool,
  ?'is_internal' => bool,
  ?'docs_url' => bool,
  ?'attributes' => vec<ExtDeclAttribute>,
);

type ExtDeclClassConst = shape(
  'name' => string,
  'type' => TypeName,
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
  'type' => TypeName,
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
  'type' => TypeName,
  ?'is_soft_type' => bool,
  ?'is_accept_disposable' => bool,
  ?'is_inout' => bool,
  ?'has_default' => bool,
  ?'is_ifc_external' => bool,
  ?'is_ifc_can_call' => bool,
  ?'is_readonly' => bool,
);

type ExtDeclSignature = shape(
  'return_type' => TypeName,
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
  'signature_type' => TypeName,
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
  'signature_type' => TypeName,
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

  ?'extends' => vec<TypeName>,
  ?'uses' => vec<TypeName>,
  ?'implements' => vec<TypeName>,
  ?'require_extends' => vec<TypeName>,
  ?'require_implements' => vec<TypeName>,
  ?'require_class' => vec<TypeName>,

  // XHP related
  ?'is_xhp' => bool,
  ?'has_xhp' => bool,
  ?'is_xhp_marked_empty' => bool,
  ?'xhp_attr_uses' => vec<TypeName>,

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

/** The FileDecls class parses a file and provides declaration information.
 */
<<__NativeData>>
final class FileDecls {

  // We want to be explicit about whether we are parsing text or file
  // The access to this class is via static methods.
  private function __construct()[] {}

  <<__Native>>
  public static function parseText(string $text)[]: FileDecls;

  <<__Native>>
  public static function parsePath(string $path)[]: FileDecls;

  <<__Native>>
  public function getError()[]: string;

  <<__Native>>
  public function hasType(string $name)[]: bool;

  <<__Native>>
  public function getFile()[]: ?ExtDeclFile;

  <<__Native>>
  public function getClasses()[]: vec<ExtDeclClass>;

  <<__Native>>
  public function getClass(string $name)[]: ?ExtDeclClass;

  <<__Native>>
  public function getFileAttributes()[]: vec<ExtDeclAttribute>;

  <<__Native>>
  public function getFileAttribute(string $name)[]: ?ExtDeclAttribute;

  <<__Native>>
  public function getFileConsts()[]: vec<ExtDeclFileConst>;

  <<__Native>>
  public function getFileConst(string $name)[]: ?ExtDeclFileConst;

  <<__Native>>
  public function getFileFuncs()[]: vec<ExtDeclFileFunc>;

  <<__Native>>
  public function getFileFunc(string $name)[]: ?ExtDeclFileFunc;

  <<__Native>>
  public function getFileModules()[]: vec<ExtDeclModule>;

  <<__Native>>
  public function getFileModule(string $name)[]: ?ExtDeclModule;

  <<__Native>>
  public function getFileTypedefs()[]: vec<ExtDeclTypedef>;

  <<__Native>>
  public function getFileTypedef(string $name)[]: ?ExtDeclTypedef;

  <<__Native>>
  public function getMethods(string $kls)[]: vec<ExtDeclMethod>;

  <<__Native>>
  public function getMethod(string $kls, string $name)[]: ?ExtDeclMethod;

  <<__Native>>
  public function getStaticMethods(string $kls)[]: vec<ExtDeclMethod>;

  <<__Native>>
  public function getStaticMethod(string $kls, string $name)[]: ?ExtDeclMethod;

  <<__Native>>
  public function getConsts(string $kls)[]: vec<ExtDeclClassConst>;

  <<__Native>>
  public function getConst(string $kls, string $name)[]: ?ExtDeclClassConst;

  <<__Native>>
  public function getTypeconsts(string $kls)[]: vec<ExtDeclTypeConst>;

  <<__Native>>
  public function getTypeconst(string $kls, string $name)[]: ?ExtDeclTypeConst;

  <<__Native>>
  public function getProps(string $kls)[]: vec<ExtDeclProp>;

  <<__Native>>
  public function getProp(string $kls, string $name)[]: ?ExtDeclProp;

  <<__Native>>
  public function getStaticProps(string $kls)[]: vec<ExtDeclProp>;

  <<__Native>>
  public function getStaticProp(string $kls, string $name)[]: ?ExtDeclProp;

  <<__Native>>
  public function getAttributes(string $kls)[]: vec<ExtDeclAttribute>;

  <<__Native>>
  public function getAttribute(string $kls, string $name)[]: ?ExtDeclAttribute;

  // Prevent cloning
  final public function __clone(): this {
    throw new \BadMethodCallException(
      'Trying to clone an uncloneable object of class FileDecls'
    );
  }

  public function __toString()[]: string {
    return __CLASS__;
  }
}
