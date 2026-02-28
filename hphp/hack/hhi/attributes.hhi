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

  // modules
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
