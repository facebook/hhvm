<?hh

/**
 * hphp_get_extension_info() - Internally used for getting extension's
 *                             information.
 *
 * @param string $name - Name of the extension
 * @return array - A map containing the extension's name, version, info string
 *                 ini settings, constants, functions and classes.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_extension_info(string $name): array<string, mixed>;

/**
 * hphp_get_method_info() - Internally used by ReflectionClass for getting a
 *                          method's information.
 *
 * @param mixed $cls   - The class which has the method, either the name or an
 *                       instance of the class.
 * @param string $name - The name of the method.
 * @return array - A map containing the method's name, access flags, modifiers,
 *                 type flags, the class name, the doc comment, the parameters,
 *                 the static variables, attributes, location in source file and
 *                 the class and method names of its prototype.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_method_info(mixed $cls, string $name): array<string, mixed>;

/**
 * hphp_get_closure_info() - Internally used by ReflectionFunction for getting
 *                           a closure's information.
 *
 * @param Closure $closure - The closure to get the information of.
 * @return array - A map containing the closure's name, closure object,
 *                 static variables, class scope and the information for the
 *                 __invoke method excluding the access flags, modifiers
 *                 and class name.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_closure_info(Closure $closure): array<string, mixed>;

/**
 * hphp_get_class_info() - Internally used by ReflectionClass for getting a
 *                         class's information.
 *
 * @param mixed $name - Either an instance or the name of the class.
 * @return array - A map containing the class's name, the extension it's from,
 *                 the name of its parent, the interfaces it implements, the
 *                 traits it uses, trait aliases, attributes, methods,
 *                 properties, constants, source information and user
 *                 attributes.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_class_info(mixed $name): array<string, mixed>;

/**
 * hphp_get_function_info() - Internally used by ReflectionFuncion for getting a
 *                            function's information.
 *
 * @param string $name - The name of the function
 * @return array - A map containing the function's name, attributes,
 *                 doc comment, parameters, static variables, user attributes
 *                 and source location.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_function_info(string $name): array<string, mixed>;

/**
 * hphp_invoke() - Used by ReflectionFunction to invoke a function.
 *
 * @param string $name        - The name of the function.
 * @param Traversable $params - The parameters to pass to the function.
 * @return mixed - The result of the invoked function.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_invoke(string $name, mixed $params): mixed;

/**
 * hphp_invoke_method() - Used by ReflectionMethod to invoke a method and by
 *                        ReflectionFunction to invoke a closure.
 *
 * @param object $obj         - An instance of the class or null for a
 *                              static method.
 * @param string $cls         - The name of the class.
 * @param string $name        - The name of the method.
 * @param Traversable $params - The parameters to pass to the method.
 * @return mixed - The result of the invoked method.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_invoke_method(?object $obj, string $cls, string $name,
                            mixed $params): mixed;

/**
 * hphp_create_object() - Used by ReflectionClass to create a new instance of an
 *                        object, including calling the constructor.
 *
 * @param string $name  - The name of the object to create.
 * @param array $params - The parameters to pass to the constructor.
 * @return object - The newly created object
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_create_object(string $name, ?array $params): object;

/**
 * hphp_create_object_without_constructor() - Used by ReflectionClass to create
 *                                            a new instance of an object,
 *                                            without calling the constructor.
 *
 * @param string $name  - The name of the object to create.
 * @return object - The newly created object
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_create_object_without_constructor(string $name): object;

/**
 * hphp_get_property() - Used by ReflectionProperty to get the value of a
 *                       property on an instance of a class.
 *
 * @param object $obj  - The object to get the property from.
 * @param string $cls  - The name of the class that the property is accessible
 *                       in or null to only get a public property.
 * @param string $prop - The name of the property.
 * @return mixed - The value of the property.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_property(
  object $obj,
  string $cls = "", // null will convert to this and do what we expect
  string $prop): mixed;

/**
 * hphp_set_property() - Used by ReflectionProperty to set the value of a
 *                       property on an instance of a class.
 *
 * @param object $obj  - The object to set the property on.
 * @param string $cls  - The name of the class that the property is accessible
 *                       in or null to only set a public property.
 * @param string $prop - The name of the property.
 * @param mixed $value - The value to set the property to.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_set_property(
  object $obj,
  string $cls = "", // null will convert to this and do what we expect
  string $prop,
  mixed $value): void;

/**
 * hphp_get_static_property() - Used by ReflectionProperty to get the value of a
 *                              static property on a class.
 *
 * @param string $cls  - The name of the class.
 * @param string $prop - The name of the static property
 * @param bool $force  - Whether or not to get protected and private properties
 *                       (true) or only public ones (false)
 * @return mixed - The value of the property
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_static_property(string $cls, string $prop,
                                  bool $force): mixed;

/**
 * hphp_set_static_property() - Used by ReflectionProperty to set the value of a
 *                              static property on a class.
 *
 * @param string $cls  - The name of the class.
 * @param string $prop - The name of the static property
 * @param mixed $value - The value to set the property to
 * @param bool $force  - Whether or not to set protected and private properties
 *                       (true) or only public ones (false)
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_set_static_property(string $cls, string $prop, mixed $value,
                                  bool $force): void;

/**
 * hphp_get_original_class_name() - Internally used by ReflectionClass for
 *                                  getting a class's declared name.
 *
 * @param string $name - The given name of the class.
 * @return string - The declared name of the class or the empty string if it
 *                  doesn't exist.
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_get_original_class_name(string $name): string;

/**
 * hphp_scalar_typehints_enabled() - Internally used by ReflectionClass for
 *                                   checking whether scalar typehints are
 *                                   enabled.
 *
 * @return bool - Whether or not scalar typehints are enabled
 */
<<__Native("NoInjection"), __HipHopSpecific>>
function hphp_scalar_typehints_enabled(): bool;
