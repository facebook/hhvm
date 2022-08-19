#include "mysql/psi/psi_error.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "mysql/components/services/psi_error_bits.h"
enum PSI_error_operation {
  PSI_ERROR_OPERATION_RAISED = 0,
  PSI_ERROR_OPERATION_HANDLED
};
typedef enum PSI_error_operation PSI_error_operation;
typedef void (*log_error_v1_t)(unsigned int error_num,
                               PSI_error_operation error_operation);
struct PSI_error_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_error_bootstrap PSI_error_bootstrap;
struct PSI_error_service_v1 {
  log_error_v1_t log_error;
};
typedef struct PSI_error_service_v1 PSI_error_service_t;
extern PSI_error_service_t *psi_error_service;
