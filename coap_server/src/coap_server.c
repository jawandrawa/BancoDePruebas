/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <zephyr/drivers/gpio.h>
#include "ot_coap_utils.h"
#include <zephyr/device.h>

//------------para latencia.--------
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
//----------------------------------

LOG_MODULE_REGISTER(coap_server, CONFIG_COAP_SERVER_LOG_LEVEL);
/*
*/
#define OT_CONNECTION_LED DK_LED1
#define PROVISIONING_LED DK_LED3
#define LIGHT_LED DK_LED4

static struct k_work provisioning_work;

static struct k_timer led_timer;
static struct k_timer provisioning_timer;

static void on_light_request(uint8_t command)
{
	static uint8_t val;

	switch (command) {
	case THREAD_COAP_UTILS_LIGHT_CMD_ON:
		dk_set_led_on(LIGHT_LED);
		val = 1;
		break;

	case THREAD_COAP_UTILS_LIGHT_CMD_OFF:
		dk_set_led_off(LIGHT_LED);
		val = 0;
		break;

	case THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE:
		val = !val;
		dk_set_led(LIGHT_LED, val);
		break;

	default:
		break;
	}
}

static void activate_provisioning(struct k_work *item)
{
	ARG_UNUSED(item);

	ot_coap_activate_provisioning();

	printk("Provisioning activated\n");
}

static void deactivate_provisionig(void)
{
	k_timer_stop(&led_timer);
	k_timer_stop(&provisioning_timer);

	if (ot_coap_is_provisioning_active()) {
		ot_coap_deactivate_provisioning();
		printk("INFO:Provisioning deactivated\n");
	}
}

static void on_provisioning_timer_expiry(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);

	deactivate_provisionig();
}

static void on_led_timer_expiry(struct k_timer *timer_id)
{
	static uint8_t val = 1;

	ARG_UNUSED(timer_id);

	dk_set_led(PROVISIONING_LED, val);
	val = !val;
}

static void on_led_timer_stop(struct k_timer *timer_id)
{
	ARG_UNUSED(timer_id);

	dk_set_led_off(PROVISIONING_LED);
}

static void on_button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & DK_BTN4_MSK) {
		k_work_submit(&provisioning_work);
	}
}

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
		}
	}
}
static struct openthread_state_changed_cb ot_state_chaged_cb = { .state_changed_cb =
									 on_thread_state_changed };


int main(void)
{
	int ret;

	printk("----------Start CoAP-server sample-----------\n");


  	//------------para latencia.--------
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set(led.port,led.pin,0);
	//----------------------------------

	//probar a comentar las siguientes tres lineas y ver funcionamiento con la placa
	k_timer_init(&led_timer, on_led_timer_expiry, on_led_timer_stop);
	k_timer_init(&provisioning_timer, on_provisioning_timer_expiry, NULL);

	k_work_init(&provisioning_work, activate_provisioning);

	ret = ot_coap_init(&deactivate_provisionig, &on_light_request,&led);
	//ret = ot_coap_init(&led);
	if (ret) {
		printk("ERROR:Could not initialize OpenThread CoAP\n");
		goto end;
	}
/*
	ret = dk_leds_init();
	if (ret) {
		printk("ERROR:Could not initialize leds, err code: %d\n", ret);
		goto end;
	}
*/
/*
	ret = dk_buttons_init(on_button_changed);
	if (ret) {
		printk("ERROR:Cannot init buttons (error: %d)\n", ret);
		goto end;
	}
*/

	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);
	openthread_start(openthread_get_default_context());

end:
	return 0;
}


