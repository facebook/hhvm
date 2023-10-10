<?hh

namespace HH {

/* Experimental section for Enum Classes. This is bound to evolve with
 * the HIP process.
 */

/**
 * Wrapper for enum class
 */
newtype MemberOf<-TEnumClass, +TType> as TType = TType;
}
