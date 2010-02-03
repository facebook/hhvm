
DEPENDENCY_ENTRY(PHPInclude)          /* PHP File      -> Container       */
DEPENDENCY_ENTRY(ClassDerivation)     /* Base Class    -> Derived Class   */
DEPENDENCY_ENTRY(FunctionCall)        /* Declaration   -> Function Call   */
DEPENDENCY_ENTRY(GlobalVariable)      /* Declaration   -> Usage           */
DEPENDENCY_ENTRY(Constant)            /* Declaration   -> Usage           */
DEPENDENCY_ENTRY(ProgramMaxInclude)   /* PHP/PHPT File -> Program         */
DEPENDENCY_ENTRY(ProgramMinInclude)   /* PHP/PHPT File -> Program         */
DEPENDENCY_ENTRY(ProgramUserFunction) /* Function      -> Program         */
DEPENDENCY_ENTRY(ProgramUserClass)    /* Class         -> Program         */
