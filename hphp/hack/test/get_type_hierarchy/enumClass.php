<?hh
enum class EC1: string {}

enum class EC2: EC1 {}
//          ^ type-hierarchy-at-caret
