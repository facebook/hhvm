<?hh

/**
 * hphp_get_extension_info() - Internally used for getting extension's
 *                             information.
 *
 * @param string $name - Name of the extension
 * @return array - A map containing the extension's name, version, info string
 *                 ini settings, constants, functions and classes.
 */
<<__Native("NoInjection")>>
function hphp_get_extension_info(string $name)[]: darray<string, mixed>;

/**
 * hphp_invoke() - Used by ReflectionFunction to invoke a function.
 *
 * @param string $name        - The name of the function.
 * @param Traversable $params - The parameters to pass to the function.
 * @return mixed - The result of the invoked function.
 */
<<__Native("NoInjection", "NoRecording")>>
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
<<__Native("NoInjection", "NoRecording")>>
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
<<__Native("NoInjection")>>
function hphp_create_object(string $name, ?varray<mixed> $params)[defaults]: object;

/**
 * hphp_create_object_without_constructor() - Used by ReflectionClass to create
 *                                            a new instance of an object,
 *                                            without calling the constructor.
 *
 * @param string $name  - The name of the object to create.
 * @return object - The newly created object
 */
<<__Native("NoInjection")>>
function hphp_create_object_without_constructor(string $name)[]: object;

/**
 * hphp_get_property() - Used by ReflectionProperty to get the value of a
 *                       property on an instance of a class.
 *
 * @param object $obj  - The object to get the property from.
 * @param string $cls  - The name of the class that the property is accessible
 *                       in
 * @param string $prop - The name of the property.
 * @return mixed - The value of the property.
 */
<<__Native("NoInjection")>>
function hphp_get_property(
  object $obj,
  string $cls,
  string $prop,
)[]: mixed;

/**
 * hphp_set_property() - Used by ReflectionProperty to set the value of a
 *                       property on an instance of a class.
 *
 * @param object $obj  - The object to set the property on.
 * @param string $cls  - The name of the class that the property is accessible
 *                       in
 * @param string $prop - The name of the property.
 * @param mixed $value - The value to set the property to.
 */
<<__Native("NoInjection")>>
function hphp_set_property(
  object $obj,
  string $cls,
  string $prop,
  mixed $value,
)[write_props]: void;

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
<<__Native("NoInjection")>>
function hphp_get_static_property(
  string $cls,
  string $prop,
  bool $force,
)[read_globals]: mixed;

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
<<__Native("NoInjection")>>
function hphp_set_static_property(
  string $cls,
  string $prop,
  mixed $value,
  bool $force,
)[globals]: void;
