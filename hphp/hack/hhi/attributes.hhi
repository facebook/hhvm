<?hh // strict

namespace HH {

// class-like
interface ClassAttribute {}
interface TypeAliasAttribute {}

// function-like
interface FunctionAttribute {}
interface MethodAttribute {}

// properties
interface PropertyAttribute {}
interface InstancePropertyAttribute extends PropertyAttribute {}
interface StaticPropertyAttribute extends PropertyAttribute {}

interface ParameterAttribute {}

}
