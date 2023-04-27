#include "mysql/psi/psi_memory.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "mysql/components/services/psi_memory_bits.h"
typedef unsigned int PSI_memory_key;
struct PSI_thread;
struct PSI_memory_info_v1 {
  PSI_memory_key *m_key;
  const char *m_name;
  unsigned int m_flags;
  int m_volatility;
  const char *m_documentation;
};
typedef struct PSI_memory_info_v1 PSI_memory_info_v1;
typedef void (*register_memory_v1_t)(const char *category,
                                     struct PSI_memory_info_v1 *info,
                                     int count);
typedef PSI_memory_key (*memory_alloc_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);
typedef PSI_memory_key (*memory_realloc_v1_t)(PSI_memory_key key,
                                              size_t old_size, size_t new_size,
                                              struct PSI_thread **owner);
typedef PSI_memory_key (*memory_claim_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);
typedef void (*memory_free_v1_t)(PSI_memory_key key, size_t size,
                                 struct PSI_thread *owner);
typedef struct PSI_memory_info_v1 PSI_memory_info;
typedef unsigned int PSI_memory_key;
struct PSI_thread;
struct PSI_memory_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_memory_bootstrap PSI_memory_bootstrap;
struct PSI_memory_service_v1 {
  register_memory_v1_t register_memory;
  memory_alloc_v1_t memory_alloc;
  memory_realloc_v1_t memory_realloc;
  memory_claim_v1_t memory_claim;
  memory_free_v1_t memory_free;
};
typedef struct PSI_memory_service_v1 PSI_memory_service_t;
extern PSI_memory_service_t *psi_memory_service;
