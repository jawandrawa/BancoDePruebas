/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include "coap_server_client_interface.h"
#include <net/coap_utils.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <zephyr/net/socket.h>
#include <openthread/thread.h>

#include "coap_client_utils.h"

LOG_MODULE_REGISTER(coap_client_utils, CONFIG_COAP_CLIENT_UTILS_LOG_LEVEL);


/* Options supported by the server */
static const char *const sensor_option[] = { SENSOR_URI_PATH, NULL };
static const char *const provisioning_option[] = { PROVISIONING_URI_PATH,
						   NULL };
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),
	.sin6_addr.s6_addr = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x54, 0x6b, 0xff, 0x3d, 0x62, 0xc8, 0x03, 0x51 },
	.sin6_scope_id = 0U
};



/* Thread multicast mesh local address 
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),
	.sin6_addr.s6_addr = { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
	.sin6_scope_id = 0U
};
*/

/* Variable for storing server address acquiring in provisioning handshake */
static char unique_local_addr_str[INET6_ADDRSTRLEN];
static struct sockaddr_in6 unique_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),
	.sin6_addr.s6_addr = {0, },
	.sin6_scope_id = 0U
};


void coap_client_utils_init(void)
{
	coap_init(AF_INET6, NULL);
	openthread_start(openthread_get_default_context());
}
