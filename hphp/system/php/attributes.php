<?hh

namespace HH {

// class-like
interface ClassLikeAttribute {}
interface ClassAttribute extends ClassLikeAttribute {}
interface ClassConstantAttribute {}
interface EnumAttribute extends ClassLikeAttribute {}
interface EnumClassAttribute extends ClassLikeAttribute {}

interface TypeAliasAttribute {}

// function-like
interface FunctionAttribute {}
interface MethodAttribute {}

interface LambdaAttribute {}

// Modules
interface ModuleAttribute {}

// properties
interface PropertyAttribute {}
interface InstancePropertyAttribute extends PropertyAttribute {}
interface StaticPropertyAttribute extends PropertyAttribute {}

interface ParameterAttribute {}
interface FileAttribute {}

interface TypeParameterAttribute {}

interface TypeConstantAttribute {}

}

namespace {

/**
 * This attribute is used by documentation generation for docs.hhvm.com to hide
 * annotated functions from having documentation generated for them. This is
 * generally because said functions are implementation details of some kind.
 */
final class NoDoc implements \HH\FunctionAttribute, HH\MethodAttribute {}

}
