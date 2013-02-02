// Get most environment from CMake, but need this for Portability.h
#if defined(__GNUC__) && defined(HAVE_FEATURES_H)
# include <features.h>
#endif
