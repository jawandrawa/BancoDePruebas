/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/pm/device.h>
#include <zephyr/net/coap.h>
#include "coap_client_utils.h"
#include <time.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/dt-bindings/clock/ast10x0_clock.h>
#include <zephyr/drivers/gpio.h>
#include "coap_server_client_interface.h"

#define ID_SENSOR 1 //TODO:cambiar para cada sensor  (1...n)
#define PERIOD_SEND_MS 5000 //TODO: periodo de envio de datos

//------para delay-------
/*
#define GPIO_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec gpio0_3 = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
*/
//-----------------------
static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),
	.sin6_addr.s6_addr = {0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			       0xf0, 0x6a, 0x6c, 0x15, 0x9d, 0xbc, 0x3d, 0x88},//TODO: añadir MLEID del CoAP Server
	.sin6_scope_id = 0U
};

/*

static struct sockaddr_in6 send_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_PORT),

	
	.sin6_addr.s6_addr = { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
	.sin6_scope_id = 0U
};

*/

static const char *const sensor_option[] = { SENSOR_URI_PATH, NULL };

//-----------------------

//para poder sacar LOGS
LOG_MODULE_REGISTER(coap_client, CONFIG_COAP_CLIENT_LOG_LEVEL);



int get_temp(void){

	int temp[5] = {15, 16, 17, 18, 19};
	return temp[rand() % 5];
}

int get_hum(void){
	return 55;
}

int get_pres(void){
	return 1000;
}

int main(void)
{
	int temp;
	int hum;
	int pres;
	char buffer[50];

	printk("-------Start CoAP-client sample------\n");

	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
		power_down_unused_ram();
	}

	coap_client_utils_init();
	//------------------ calculo latencia----------
	/*
	gpio_pin_configure_dt(&gpio0_3,GPIO_OUTPUT_ACTIVE);
	gpio_pin_set(gpio0_3.port,gpio0_3.pin,0);
	//---------------------------------------------
	*/
	while(1){
		temp = get_temp();
		hum = get_hum();
		char buffer[50];
		
    	memset(buffer, 0, sizeof(buffer));
		
		snprintf(buffer, sizeof(buffer),"ID:%d,t:%d,h:%d\n",ID_SENSOR, temp, hum);

		LOG_INF(" %s\n",buffer);

		//-----para latencia-------
		/*
		gpio_pin_set(gpio0_3.port,gpio0_3.pin,1);
		//--------------------------
		*/

		//funcion que envia datos
		coap_send_request(COAP_METHOD_PUT,
				  (const struct sockaddr *)&send_local_addr,
				  sensor_option, buffer,sizeof(buffer), NULL);

		//-----para latencia-------
		/*
		gpio_pin_set(gpio0_3.port,gpio0_3.pin,0);
		//--------------------------
		*/
		k_sleep(K_MSEC(PERIOD_SEND_MS));
	}

	return 0;
}

