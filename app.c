/***************************************************************************//**
 * @file
 * @brief Empty NCP-host Example Project.
 *
 * Reference implementation of an NCP (Network Co-Processor) host, which is
 * typically run on a central MCU without radio. It can connect to an NCP via
 * VCOM to access the Bluetooth stack of the NCP and to control it using BGAPI.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include "app.h"
#include "gatt_db.h"
#include "ncp_host.h"
#include "app_log.h"
#include "app_log_cli.h"
#include "app_assert.h"
#include "sl_bt_api.h"

// Optstring argument for getopt.
#define OPTSTRING      NCP_HOST_OPTSTRING APP_LOG_OPTSTRING "h"

// Usage info.
#define USAGE          APP_LOG_NL "%s " NCP_HOST_USAGE APP_LOG_USAGE " [-h]" APP_LOG_NL

// Options info.
#define OPTIONS    \
  "\nOPTIONS\n"    \
  NCP_HOST_OPTIONS \
  APP_LOG_OPTIONS  \
  "    -h  Print this help message.\n"





static uint8_t find_service_in_advertisement(uint8_t *data, uint8_t len);
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };


volatile int BleADVCounter = 0;
volatile int counter = 0;
uint8_t 	connection_handle_;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(int argc, char *argv[])
{
  sl_status_t sc;
  int opt;

  // Process command line options.
  while ((opt = getopt(argc, argv, OPTSTRING)) != -1) {
    switch (opt) {
      // Print help.
      case 'h':
        app_log(USAGE, argv[0]);
        app_log(OPTIONS);
        exit(EXIT_SUCCESS);

      // Process options for other modules.
      default:
        sc = ncp_host_set_option((char)opt, optarg);
        if (sc == SL_STATUS_NOT_FOUND) {
          sc = app_log_set_option((char)opt, optarg);
        }
        if (sc != SL_STATUS_OK) {
          app_log(USAGE, argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
    }
  }

 

  // Initialize NCP connection.
  sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  app_assert_status(sc);
  app_log_info("NCP host initialised." APP_LOG_NL);
  app_log_info("Resetting NCP target..." APP_LOG_NL);
  // Reset NCP to ensure it gets into a defined state.
  // Once the chip successfully boots, boot event should be received.
  sl_bt_system_reset(sl_bt_system_boot_mode_normal);

  app_log_info("Press Crtl+C to quit" APP_LOG_NL APP_LOG_NL);

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Deinit.
 *****************************************************************************/
void app_deinit(void)
{
  ncp_host_deinit();

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application deinit code here!                       //
  // This is called once during termination.                                 //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t	connection_handle;
  //bd_addr address;
  //uint8_t address_type;
  //uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

       sc = sl_bt_scanner_start(sl_bt_scanner_scan_phy_1m,
                               sl_bt_scanner_discover_generic);
      app_assert_status(sc);
      app_log("Scanning started\r\n");


      break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id:



        if (evt->data.evt_scanner_legacy_advertisement_report.event_flags
          == (SL_BT_SCANNER_EVENT_FLAG_CONNECTABLE | SL_BT_SCANNER_EVENT_FLAG_SCANNABLE)) {

        
        // If a thermometer advertisement is found...
        if (find_service_in_advertisement(&(evt->data.evt_scanner_legacy_advertisement_report.data.data[0]),
                                          evt->data.evt_scanner_legacy_advertisement_report.data.len) != 0) {

          app_log("If a thermometer advertisement is found...\n\r");

          printf("BLE device found\r\n");
          printf (" rssi:%d  \r\n", evt->data.evt_scanner_legacy_advertisement_report.rssi );
          printf (" address: %x6s \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.address.addr);
          printf (" address_type: %d  \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.address_type);
          printf (" bonding: %d  \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.bonding);
          printf (" data:%xs \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.data.data);
          printf("Advertisement Count: %d\r\n", BleADVCounter++ );

          // then stop scanning for a while
          sc = sl_bt_scanner_stop();
          app_assert_status(sc);
          // and connect to that device
          app_log("Opening connection\n\r");


        sc = sl_bt_connection_open (evt->data.evt_scanner_legacy_advertisement_report.address, 
                                    evt->data.evt_scanner_legacy_advertisement_report.address_type, 
                                    sl_bt_gap_phy_1m, &connection_handle_);
         
          app_assert_status(sc);

          app_log("Opening process\n\r");
        }
      }
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log("Connection opened\n\r");

      connection_handle = evt->data.evt_connection_opened.connection;

     
      break;

    case sl_bt_evt_gatt_service_id:
      //app_log("Services Discovered\n\r");
      app_log("Service Handle : %d Service UUID: %s \n\r",evt->data.evt_gatt_service.service, evt->data.evt_gatt_service.uuid);

      break;

    case sl_bt_evt_connection_parameters_id:
     app_log("Connection Parameters exchange\n\r");

     sl_bt_system_set_lazy_soft_timer(3*32768, 0, 0xaa,1);
      
    if (evt->data.evt_connection_parameters.txsize > 27)
    {
      app_log("Connection Parameters Set\n\r");
    }
    break;
    
    case sl_bt_evt_system_soft_timer_id:

    sc = sl_bt_gatt_discover_primary_services(connection_handle); //sl_bt_gatt_discover_primary_services(evt->data.evt_connection_parameters.connection);
       app_log(" Softimer SC=  %x \n\r", sc);
    app_assert_status(sc);
    
    break;

    case sl_bt_evt_gatt_procedure_completed_id:

    sc = evt->data.evt_gatt_procedure_completed.result;
    app_log(" sl_bt_evt_gatt_procedure_completed_id %x \n\r", sc);
    app_assert_status(sc);

    break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("Connection closed." APP_LOG_NL);

      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}


// Parse advertisements looking for advertised Health Thermometer service
static uint8_t find_service_in_advertisement(uint8_t *data, uint8_t len)
{
  uint8_t ad_field_length;
  uint8_t ad_field_type;
  uint8_t i = 0;
  // Parse advertisement packet
  while (i < len) {
    ad_field_length = data[i];
    ad_field_type = data[i + 1];
    // Partial ($02) or complete ($03) list of 16-bit UUIDs
    if (ad_field_type == 0x02 || ad_field_type == 0x03) {
      // compare UUID to Health Thermometer service UUID
      if (memcmp(&data[i + 2], thermo_service, 2) == 0) {
        return 1;
      }
    }
    // advance to the next AD struct
    i = i + ad_field_length + 1;
  }
  return 0;
}