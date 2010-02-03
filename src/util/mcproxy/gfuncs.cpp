/*
 * Guarded string and memory manipulation functions.
 */

#include "gfuncs.h"

static void *
guard(void *result)
{
  if (result == NULL)
  {
    HPHP::Logger::Error("Out of memory!  Aborting.");
                throw HPHP::Exception("abort");
  }

  return result;
}


void *
gmalloc(int size)
{
  return guard(
#ifdef USE_GC
    GC_MALLOC(size)
#else
    malloc(size)
#endif
    );
}

void *
gmalloc_data(int size)
{
  return guard(
#ifdef USE_GC
    GC_MALLOC_ATOMIC(size)
#else
    malloc(size)
#endif
    );
}

void *
grealloc(void *orig, int size)
{
  if (orig == NULL)
    return gmalloc(size);
  else
    return guard(
#ifdef USE_GC
      GC_REALLOC(orig, size)
#else
      realloc(orig, size)
#endif
    );
}

char *
gstrdup(const char *str)
{
  if (str == NULL)
    return NULL;

#ifdef USE_GC
  {
    char *dup = guard(GC_MALLOC_ATOMIC(strlen(str) + 1));
    strcpy(dup, str);
    return dup;
  }
#else
# ifdef FIND_LEAKS
  {
    char *dup = gmalloc(strlen(str) + 1);
    strcpy(dup, str);
    return dup;
  }
# else
  return (char *) guard(strdup(str));
# endif /* FIND_LEAKS */
#endif /* USE_GC */
}

char *
gstrdup_const(const char *str)
{
  if (str == NULL)
    return NULL;

#ifdef USE_GC
  return (char *) str;
#else
  return gstrdup(str);
#endif
}

void *
gmemdup_const(const char *data, unsigned int bytes)
{
  if (data == NULL)
    return NULL;

#ifdef USE_GC
  return (char *) data;
#else
  {
    char *newdata = (char*)gmalloc(bytes);
    memcpy(newdata, data, bytes);
    return newdata;
  }
#endif
}

void *
gcalloc(int size)
{
#ifdef USE_GC
  return gmalloc(size);
#else
  return guard(calloc(1, size));
#endif
}

void *
gcalloc_data(int size)
{
#ifdef USE_GC
  return gmalloc_data(size);
#else
  return gcalloc(size);
#endif
}

int
gstrlen(const char *str)
{
  if (str == NULL)
    return 0;
  return strlen(str);
}

/* strncpy that always null-terminates */
void
gstrncpy(char *dest, const char *src, int maxlen)
{
  if (maxlen == 0 || src == NULL || dest == NULL)
    return;
  while (--maxlen && *src != '\0')
    *dest++ = *src++;
  *dest = '\0';
}

/* atoi that handles null pointers */
int
gatoi(const char *str)
{
  if (str == NULL)
    return 0;
  return atoi(str);
}

int
endswith(const char *str, const char *suffix)
{
  if (str == NULL || suffix == NULL)
    return 0;
  if (strlen(str) < strlen(suffix))
    return 0;

  return ! strcmp(str + strlen(str) - strlen(suffix), suffix);
}

void
gfree(void *ptr)
{
#ifdef USE_GC
  /* Let garbage collector do its thing */
#else
  if (ptr != NULL)
    free(ptr);
#endif
}

void ginit(void)
{
#ifdef USE_GC
  GC_INIT();
#endif
}
