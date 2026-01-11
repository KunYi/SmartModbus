/**
 * @file mb_transport.h
 * @brief Transport layer abstraction for Smart Modbus library
 *
 * This file defines the transport layer interface that allows the library
 * to work with different physical layers (UART, TCP socket, etc.).
 */

#ifndef SMARTMODBUS_MB_TRANSPORT_H
#define SMARTMODBUS_MB_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transport layer callbacks
 *
 * Users must implement these callbacks to interface with their specific
 * hardware or network layer.
 */
typedef struct {
    /**
     * @brief Send data through transport layer
     * @param ctx User context pointer
     * @param data Data buffer to send
     * @param len Length of data in bytes
     * @return Number of bytes sent, or negative error code
     */
    int (*send)(void *ctx, const uint8_t *data, size_t len);

    /**
     * @brief Receive data from transport layer
     * @param ctx User context pointer
     * @param buffer Buffer to store received data
     * @param max_len Maximum buffer size
     * @param received Pointer to store actual bytes received
     * @return 0 on success, negative error code on failure
     */
    int (*recv)(void *ctx, uint8_t *buffer, size_t max_len, size_t *received);

    /**
     * @brief Delay for specified number of character times (optional)
     * @param ctx User context pointer
     * @param chars Number of character times to delay
     *
     * This is primarily used for RTU/ASCII inter-frame gap timing.
     * For TCP/IP, this can be NULL.
     */
    void (*delay_chars)(void *ctx, uint16_t chars);

    /**
     * @brief User context pointer
     *
     * This pointer is passed to all callback functions and can be used
     * to store transport-specific state (e.g., file descriptor, UART handle).
     */
    void *context;
} mb_transport_t;

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_MB_TRANSPORT_H
