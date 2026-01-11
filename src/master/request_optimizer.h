/**
 * @file request_optimizer.h
 * @brief Request optimization pipeline orchestrator
 */

#ifndef SMARTMODBUS_REQUEST_OPTIMIZER_H
#define SMARTMODBUS_REQUEST_OPTIMIZER_H

#include "smartmodbus/mb_config.h"
#include "smartmodbus/mb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main optimization pipeline
 * @param request User read request
 * @param config Configuration
 * @param plans Output array of optimized request plans
 * @param max_plans Maximum number of plans
 * @param plan_count Output: actual number of plans created
 * @return 0 on success, negative error code on failure
 */
int mb_optimize_request(const mb_read_request_t *request,
                        const mb_config_t *config,
                        mb_request_plan_t *plans,
                        uint16_t max_plans,
                        uint16_t *plan_count);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_REQUEST_OPTIMIZER_H
