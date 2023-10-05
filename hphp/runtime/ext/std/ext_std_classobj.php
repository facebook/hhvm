<?hh // partial

namespace {

/**
 * Checks if the class has been defined
 *
 * @param string $class_name - The class name. The name is matched in a
 *   case-insensitive manner.
 * @param bool $autoload - Whether or not to call by default.
 *
 * @return bool - Returns TRUE if class_name is a defined class, FALSE
 *   otherwise.
 */
<<__Native>>
function class_exists(string $class_name,
                      bool $autoload = true)[]: bool;

/**
 * Get the constants of the given class.
 *
 * @param string $class_name - The class name
 *
 * @return array - Returns an associative array of constants with their values.
 */
<<__Native>>
function get_class_constants(string $class_name)[]: darray<string, mixed>;

/**
 * Gets the class methods' names
 *
 * @param mixed $class_name - The class name or an object instance
 *
 * @return array - Returns an array of method names defined for the class
 *   specified by class_name. In case of an error, it returns NULL.
 */
<<__Native>>
function get_class_methods(readonly mixed $class_name)[]: ?varray<string>;

/**
 * Get the default properties of the class
 *
 * @param string $class_name - The class name
 *
 * @return array - Returns an associative array of the declared properties
 *   visible from the current scope to their appropriate values. Values for
 *   instance property keys are their default value, and values for static
 *   properties are their current values. The resulting array elements are in
 *   the form of varname => value. In case of an error, it returns FALSE.
 */
<<__Native>>
function get_class_vars(string $class_name): mixed;

/**
 * Returns the name of the class of an object
 *
 * @param object $object - The tested object. This parameter may be
 *   omitted when inside a class.
 *
 * @return string - Returns the name of the class of which object is an
 *   instance. Returns FALSE if object is not an object.   If object is
 *   omitted when inside a class, the name of that class is returned.
 */
<<__Native("NoRecording")>>
function get_class(readonly mixed $object = null)[]: mixed;

/**
 * Returns an array with the name of the defined classes
 *
 * @return array - Returns an array of the names of the declared classes
 *   in the current script.    Note that depending on what extensions you
 *   have compiled or loaded into PHP, additional classes could be present.
 *   This means that you will not be able to define your own classes using
 *   these names. There is a list of predefined classes in the Predefined
 *   Classes section of the appendices.
 */
<<__Native>>
function get_declared_classes(): varray<string>;

/**
 * Returns an array of all declared interfaces
 *
 * @return array - Returns an array of the names of the declared
 *   interfaces in the current script.
 */
<<__Native>>
function get_declared_interfaces(): varray<string>;

/**
 * Returns an array of all declared traits
 *
 * @return array - Returns an array with names of all declared traits in
 *   values. Returns NULL in case of a failure.
 */
<<__Native>>
function get_declared_traits(): varray<string>;

/**
 * Gets the properties of the given object
 *
 * @param object $object - An object instance.
 *
 * @return array - Returns an associative array of defined object
 *   accessible non-static properties for the specified object in scope. If
 *   a property has not been assigned a value, it will be returned with a
 *   NULL value.
 */
<<__Native>>
function get_object_vars(object $object)[]: darray<string, mixed>;

/**
 * Retrieves the parent class name for object or class
 *
 * @param mixed $object - The tested object or class name
 *
 * @return string - Returns the name of the parent class of the class of
 *   which object is an instance or the name.    If the object does not
 *   have a parent or the class given does not exist FALSE will be
 *   returned.    If called without parameter outside object, this function
 *   returns FALSE.
 */
<<__Native>>
function get_parent_class(readonly mixed $object = null)[]: mixed;

/**
 * Checks if the interface has been defined
 *
 * @param string $interface_name - The interface name
 * @param bool $autoload - Whether to call or not by default.
 *
 * @return bool - Returns TRUE if the interface given by interface_name
 *   has been defined, FALSE otherwise.
 */
<<__Native>>
function interface_exists(string $interface_name,
                          bool $autoload = true)[]: bool;

/**
 * Checks if the object is of this class or has this class as one of its
 * parents
 *
 * @param object $object - The tested object
 * @param string $class_name - The class name
 * @param bool $allow_string - If this parameter set to FALSE, string
 *   class name as object is not allowed. This also prevents from calling
 *   autoloader if the class doesn't exist.
 *
 * @return bool - Returns TRUE if the object is of this class or has this
 *   class as one of its parents, FALSE otherwise.
 */
<<__Native>>
function is_a(readonly mixed $object,
              string $class_name,
              bool $allow_string = false)[]: bool;

/**
 * Checks if the object has this class as one of its parents
 *
 * @param mixed $object - A class name or an object instance
 * @param string $class_name - The class name
 * @param bool $allow_string - If this parameter set to false, string
 *   class name as object is not allowed. This also prevents from calling
 *   autoloader if the class doesn't exist.
 *
 * @return bool - This function returns TRUE if the object object,
 *   belongs to a class which is a subclass of class_name, FALSE otherwise.
 */
<<__Native>>
function is_subclass_of(readonly mixed $object,
                        string $class_name,
                        bool $allow_string = true)[]: bool;

/**
 * Checks if the class method exists
 *
 * @param mixed $object - An object instance or a class name
 * @param string $method_name - The method name
 *
 * @return bool - Returns TRUE if the method given by method_name has
 *   been defined for the given object, FALSE otherwise.
 */
<<__Native>>
function method_exists(readonly mixed $object,
                       string $method_name)[]: bool;

/**
 * Checks if the object or class has a property
 *
 * @param mixed $class - The class name or an object of the class to test
 *   for
 * @param string $property - The name of the property
 *
 * @return bool - Returns TRUE if the property exists, FALSE if it
 *   doesn't exist or NULL in case of an error.
 */
<<__Native>>
function property_exists(readonly mixed $class,
                         string $property)[]: ?bool;

/**
 * Checks if the trait exists
 *
 * @param string $traitname -
 * @param bool $autoload -
 *
 * @return bool - Returns TRUE if trait exists, FALSE if not, NULL in
 *   case of an error.
 */
<<__Native>>
function trait_exists(string $traitname,
                      bool $autoload = true)[]: bool;

/**
 * Checks if the enum exists
 *
 * @param string $enumname -
 * @param bool $autoload -
 *
 * @return bool - Returns TRUE if enum exists, FALSE if not
 */
<<__Native>>
function enum_exists(string $enumname,
                      bool $autoload = true)[]: bool;

/**
 * Checks if the module exists
 *
 * @param string $modulename -
 * @param bool $autoload -
 *
 * @return bool - Returns TRUE if module exists, FALSE if not
 */
<<__Native>>
function module_exists(string $modulename,
                       bool $autoload = true)[]: bool;
}

namespace HH {

/**
 * Get class name from class_meth
 * @param mixed $class_meth
 * @return class name
 */
<<__Native>>
function class_meth_get_class(readonly mixed $class_meth)[]: string;

/**
 * Get class name from class
 * @param mixed $class
 * @return class name
 */
<<__Native>>
function class_get_class_name(readonly mixed $class)[]: string;

/**
 * Get method name from class_meth
 * @param mixed $class_meth
 * @return method name
 */
<<__Native>>
function class_meth_get_method(readonly mixed $class_meth)[]: string;

/**
 * Get class name from meth_caller
 * @param mixed $meth_caller
 * @return class name
 */
<<__Native, __IsFoldable>>
function meth_caller_get_class(readonly mixed $meth_caller)[]: string;

/**
 * Get method name from meth_caller
 * @param mixed $meth_caller
 * @return method name
 */
<<__Native, __IsFoldable>>
function meth_caller_get_method(readonly mixed $meth_caller)[]: string;
}
