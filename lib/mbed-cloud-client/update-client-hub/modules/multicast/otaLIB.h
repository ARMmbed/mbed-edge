// ----------------------------------------------------------------------------
// Copyright 2020 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

// OTA library user interface header file

#ifndef OTALIB_H_
#define OTALIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * \file otaLIB.h
 * \brief OTA Library API. OTA (Over-The-Air programming) library takes care of delivering (in router case) and
 *        receiving (in both router and node cases) firmware.
 *
 *  \section ota-api OTA Library API:
 *  - ota_lib_configure(), A function to configure OTA library.
 *
 *  \section ota-api usage:
 *  1. Configure OTA library by ota_lib_configure()
 *
 */

#define OTA_FW_NAME_OR_VERSION_MAX_LENGTH 64 // In bytes

#define OTA_WHOLE_FW_CHECKSUM_LENGTH 32 // In bytes

// * * * Fragments request bitmask length * * *
#define OTA_FRAGMENTS_REQ_BITMASK_LENGTH 16 // In bytes (bitmask consist of 128 bits: 16 bytes * 8 bits = 128 bits)

// * * * Forward declarations * * *
typedef struct ota_server_s ota_server_t;
extern const ota_server_t ota_server_ref;
typedef uint8_t ota_coap_callback_t(struct nsdl_s *, sn_coap_hdr_s *, sn_nsdl_addr_s *, sn_nsdl_capab_e);

#define OTA_SERVER (&ota_server_ref)
#define OTA_CLIENT NULL

// * * * Enums * * *

/*!
 * \enum ota_device_type_e
 * \brief Enum for Device types.
 */
typedef enum ota_device_type_e
{
  OTA_DEVICE_TYPE1 = 1, // E.g. Border router type 1
  OTA_DEVICE_TYPE2 = 2, // E.g. Border router type 2
  OTA_DEVICE_TYPE3 = 3, // E.g. node type 1
  OTA_DEVICE_TYPE4 = 4, // E.g. node type 2
  OTA_DEVICE_TYPE5 = 5  // E.g. node type 3

} ota_device_type_e;

/*!
 * \enum ota_process_state_e
 * \brief Enum for OTA process state.
 */
typedef enum ota_process_state_e
{
  // OTA_START_CMD command is received and firmware fragments are ready to receive
  OTA_STATE_STARTED = 1,

  // OTA_ABORT_CMD command is received, continued with OTA_START_CMD
  OTA_STATE_ABORTED = 2,

  // Device is missing fragments and is requesting those
  OTA_STATE_MISSING_FRAGMENTS_REQUESTING = 3,

  // Checksum calculating over whole received firmware is ongoing
  OTA_STATE_CHECKSUM_CALCULATING = 4,

  // Checksum calculating over whole received firmware is failed
  OTA_STATE_CHECKSUM_FAILED = 5,

  // All firmware fragments are received
  OTA_STATE_PROCESS_COMPLETED = 6,

  // Waiting device's reset by application for taking new downloaded own firmware in use
  OTA_STATE_UPDATE_FW = 7,

  // Error state
  OTA_STATE_INVALID = 8

} ota_process_state_e;

/*!
 * \enum ota_error_code_e
 * \brief Enum for ok/error codes.
 */
typedef enum ota_error_code_e
{
  // Ok value
  OTA_OK = 0,

  // Error values
  OTA_STORAGE_ERROR = 1,
  OTA_STORAGE_OUT_OF_SPACE = 2,
  OTA_PARAMETER_FAIL = 3,
  OTA_FUNC_PTR_NULL = 4,
  OTA_OUT_OF_MEMORY = 5,
  OTA_RESOURCE_CREATING_FAILED = 6

} ota_error_code_e;

/*!
 * \enum ota_address_type_e
 * \brief Enum for address type.
 */
typedef enum ota_address_type_e
{
  OTA_ADDRESS_NOT_VALID = 0,

  OTA_ADDRESS_IPV6 = 1,
  OTA_ADDRESS_IPV4 = 2

} ota_address_type_e;

// * * * Structs * * *

/*!
 * \struct ota_ip_address_t
 * \brief IP address structure.
 */
typedef struct ota_ip_address_t
{
  ota_address_type_e type; // Address type, see ota_address_type_e
  uint8_t address_tbl[16]; // IP address
  uint16_t port;           // UDP port number

} ota_ip_address_t;

/*!
 * \struct ota_lib_config_data_t
 * \brief OTA library configuring structure.
 */
typedef struct ota_lib_config_data_t
{
  uint8_t device_type;
  uint8_t ota_max_processes_count;

  sn_coap_msg_type_e response_msg_type; // OTA library CoAP response type: COAP_MSG_TYPE_CONFIRMABLE or COAP_MSG_TYPE_NON_CONFIRMABLE

  uint16_t response_sending_delay_start; // Response sending random delay start value in seconds (end time is given in START command)
                                         // Note: This value is directly (no random) used in unicast case before sending response
  ota_ip_address_t unicast_socket_addr;              // Unicast socket address
  ota_ip_address_t link_local_multicast_socket_addr; // Link local multicast socket address
  ota_ip_address_t mpl_multicast_socket_addr;        // MPL multicast socket address
} ota_lib_config_data_t;

/*!
 * \struct ota_processes_t
 * \brief Used for storing OTA processes over reset.
 */
typedef struct ota_processes_t
{
  uint8_t ota_process_count; // Tells how many OTA processes are active (only in router, in node always one)

  uint32_t *ota_process_ids_tbl;
} ota_processes_t;

/*!
 * \struct ota_download_state_t
 * \brief Used for storing OTA state over reset.
 */
typedef struct ota_download_state_t
{
  uint32_t ota_process_id; // OTA process ID
  ota_process_state_e ota_state; // OTA process state

  uint16_t fragments_bitmask_length; // Received and stored fragments bitmask length in bytes
  uint8_t *fragments_bitmask_ptr;    // Received and stored fragments bitmask. One bit is for one fragment: If bit is 0, fragment is not received.
                                     // Bit order e.g. for segment 1: (fragment 128) MSB...LSB (fragment 1)
} ota_download_state_t;

/*!
 * \struct ota_parameters_t
 * \brief Used for storing OTA parameters over reset.
 */
typedef struct ota_parameters_t
{
  uint32_t ota_process_id; // OTA process ID

  uint8_t device_type; // Device type

  uint16_t response_sending_delay_start; // Response sending random delay min value in seconds
  uint16_t response_sending_delay_end;   // Response sending random delay max value in seconds

  uint16_t fw_download_report_config; // Firmware download state report configuration:
                                      //  0: No automatic reporting by OTA library
                                      //  Other value: Automatic reporting period in seconds

  bool multicast_used_flag; // Unicast/Multicast used

  uint8_t fw_name_length; // Firmware name length as bytes
  uint8_t *fw_name_ptr;   // Firmware name

  uint8_t fw_version_length; // Firmware version length as bytes
  uint8_t *fw_version_ptr;   // Firmware version

  uint16_t fw_segment_count; // Firmware segment count

  uint32_t fw_total_byte_count;    // Firmware total btye count

  uint16_t fw_fragment_count;                // Firmware fragment count
  uint16_t fw_fragment_byte_count;           // Byte count in one firmware fragment
  uint16_t fw_fragment_sending_interval_uni; // Firmware fragment sending interval in millisecondsf for Unicast and Link local multicast
  uint16_t fw_fragment_sending_interval_mpl; // Firmware fragment sending interval in milliseconds for MPL multicast

  uint8_t whole_fw_checksum_tbl[OTA_WHOLE_FW_CHECKSUM_LENGTH]; // Whole firmware image checksum

  uint8_t fallback_timeout; // Value in hours. After this timeout, device will independently start requesting its missing fragments.

  ota_ip_address_t missing_fragments_req_addr; // OPTIONAL: Missing fragments requesting address

  uint8_t delivered_image_resource_name_length; // Only for router: Delivered image resource name length
  uint8_t *delivered_image_resource_name_ptr;   // Only for router: Delivered image resource name

  uint8_t pull_url_length;                      // Only for router: Url where to pull firmware image length
  uint8_t* pull_url_ptr;                        // Only for router: Url where to pull firmware image
} ota_parameters_t;

/*!
 * \struct ota_config_func_pointers_t
 * \brief OTA function pointer configuring structure.
 * Structure is used for configuring OTA.
 */
typedef struct ota_config_func_pointers_t
{
  void *(*mem_alloc_fptr)(size_t);
  void (*mem_free_fptr)(void*);

  void (*request_timer_fptr)(uint8_t, uint32_t);
  void (*cancel_timer_fptr)(uint8_t);

  ota_error_code_e (*store_new_ota_process_fptr)(uint32_t);
  ota_error_code_e (*read_stored_ota_processes_fptr)(ota_processes_t*);
  ota_error_code_e (*remove_stored_ota_process_fptr)(uint32_t);

  ota_error_code_e (*store_state_fptr)(ota_download_state_t*);
  ota_error_code_e (*read_state_fptr)(uint32_t, ota_download_state_t*);

  ota_error_code_e (*store_parameters_fptr)(ota_parameters_t*);
  ota_error_code_e (*read_parameters_fptr)(uint32_t, ota_parameters_t*);

  ota_error_code_e (*start_received_fptr)(ota_parameters_t*);
  void (*process_finished_fptr)(uint32_t);

  uint32_t (*get_fw_storing_capacity_fptr)(void);
  uint32_t (*write_fw_bytes_fptr)(uint32_t, uint32_t, uint32_t, uint8_t*);
  uint32_t (*read_fw_bytes_fptr)(uint32_t, uint32_t, uint32_t, uint8_t*);

  void (*send_update_fw_cmd_received_info_fptr)(uint32_t, uint16_t);

  void (*update_device_registration_fptr)(void);

  int8_t (*socket_send_fptr)(ota_ip_address_t*, uint16_t, uint8_t*);

  uint16_t (*coap_send_notif_fptr)(char *, uint8_t*, uint16_t);

  ota_error_code_e (*create_resource_fptr)(const char *, const char *,int32_t, bool, ota_coap_callback_t *, bool);

} ota_config_func_pointers_t;

// * * * Function prototypes * * *

/**
 * \brief A function for configuring OTA library
 * \param nsdl_handle_ptr NSDL handle for handling resources
 * \param lib_config_data_ptr OTA configuration data
 * \param func_pointers_ptr Function pointers for OTA library:
 *          mem_alloc_fptr() Function pointer for allocating memory
 *            Parameters:
 *              -Allocated byte count
 *            Return value:
*              -Data pointer to allocated memory, NULL in error case
 *          mem_free_fptr() Function pointer for freeing allocated memory
 *            Parameters:
 *              -Data pointer to freed memory
 *          request_timer_fptr() Function pointer for requesting timer event
 *            Parameters:
 *              -Timer ID of requested timer
 *              -Timeout time in milliseconds
 *          cancel_timer_fptr() Function pointer for canceling requested timer event
 *            Parameters:
 *              -Timer ID of cancelled timer
 *          store_new_ota_process_fptr() Function pointer for storing new OTA process to storage
 *            Parameters:
 *              -Added OTA process ID
 *            Return value:
 *              -Ok/error status code of performing function
 *          read_stored_ota_processes_fptr() Function pointer for reading stored OTA processes from storage
 *            Parameters:
 *              -Stored OTA processes
 *            Return value:
 *              -Ok/error status code of performing function
 *          remove_stored_ota_process_fptr() Function pointer for removing stored OTA process from storage
 *            Parameters:
 *              -Removed OTA process ID
 *            Return value:
 *              -Ok/error status code of performing function
 *          store_state_fptr() Function pointer for OTA library for storing one OTA process state to storage managed by application
 *            Parameters:
 *              -Stored OTA state
 *            Return value:
 *              -Ok/error status code of performing function
 *          read_state_fptr() Function pointer for OTA library for reading one OTA process state from storage managed by application
 *            Parameters:
 *              -OTA process ID for selecting which OTA process state is read
 *              -Data pointer where OTA process state is read
 *               NOTE: OTA library user (OTA application) will allocate memory for data pointers of ota_download_state_t and
 *                     OTA library will free these data pointers with free function given in configure
 *            Return value:
 *              -Ok/error status code of performing function
 *          store_parameters_fptr() Function pointer for storing OTA parameters to storage
 *            Parameters:
 *              -Stored OTA parameters
 *            Return value:
 *              -Ok/error status code of performing function
 *          read_parameters_fptr() Function pointer for reading one OTA process parameters from storage
 *            Parameters:
 *              -OTA process ID for selecting which OTA process parameters are read
 *              -Data pointer where OTA process parameters are read
 *               NOTE: OTA library user (OTA application) will allocate memory for data pointers of ota_parameters_t and
 *                     OTA library will free these data pointers with free function given in configure
 *            Return value:
 *              -Ok/error status code of performing function
 *          get_fw_storing_capacity_fptr() Function pointer for getting byte count of firmware image storing storage
 *            Return value:
 *              -Byte count of total storage for firmware images
 *          write_fw_bytes_fptr() Function pointer for writing firmware bytes to storage
 *            Parameters:
 *              -OTA process ID
 *              -Byte offset (tells where data to be written)
 *              -To be written data byte count
 *              -Data pointer to be written data
 *            Return value:
 *              -Written byte count
 *          read_fw_bytes_fptr() Function pointer for reading firmware bytes from storage
 *            Parameters:
 *              -OTA process ID
 *              -Byte offset (tells where data is to read)
 *              -Data byte count to be read
 *              -Data pointer to data to be read
 *            Return value:
 *              -Read byte count
 *          send_update_fw_cmd_received_info_fptr() Function pointer for telling to application that firmware image can be taken in use
 *                                                  NOTE: OTA user (backend) must remove OTA process from data storage with DELETE command
 *            Parameters:
 *              -OTA process ID
 *              -Delay time in seconds before taking new firmware in use
 *          update_device_registration_fptr() Function pointer for device's registration for getting dynamic resource registered right away
 *                                            NOTE: This function pointer can be set to NULL if not wanted to use
 *          socket_send_fptr() Function pointer for sending data to socket
 *            Parameters:
 *              -Sent destination address
 *              -Sent payload length
 *            Return value:
 *              -Success: 0
 *              -Invalid socket ID: -1
 *              -Socket memory allocation fail: -2
 *              -TCP state not established: -3
 *              -Socket TX process busy: -4
 *              -Packet too short: -6
 *          coap_send_notif_fptr() Function pointer for sending notifications via CoAP
 *            Parameters:
 *              -Sent destination address
 *              -Pointer to token to be used
 *              -Token length
 *              -Pointer to payload to be sent
 *              -Payload length
 *              -Pointer to observe number to be sent
 *              -Observe number len
 *              -Observation message type (confirmable or non-confirmable)
 *            Return value:
 *              -Success, observation messages message ID: !0
 *              -Failure: 0
 * \param max_process_count Maximum amount of concurrent processes allowed, Should be always 1 for node.
 * \param server Pass OTA_CLIENT for client usage, OTA_SERVER for server usage
 * \return Ok/error status code of performing function, see ota_error_code_e
 */
extern ota_error_code_e ota_lib_configure(ota_lib_config_data_t *lib_config_data_ptr,
                                          ota_config_func_pointers_t *func_pointers_ptr,
                                          uint8_t max_process_count, const ota_server_t *server_ptr);

/**
 * @brief ota_lib_reset Resets OTA lib and frees allocated resources (if any)
 */
extern void ota_lib_reset();

/**
 * \brief A function which must be called by application for OTA library's expired timers
 * \param timer_id Timer ID
 * \return Ok/error status code of performing function, see ota_error_code_e
 */
void ota_timer_expired(uint8_t timer_id);

/**
 * \brief A function which must be called by application for OTA library's received OTA socket data
 * \param payload_length Received payload length
 * \param payload_ptr Received payload data
 * \param source_addr_ptr Source address of received socket data
 */
void ota_socket_receive_data(uint16_t payload_length,
                             uint8_t *payload_ptr,
                             ota_ip_address_t *source_addr_ptr);

/**
 * \brief A function which set download process to completed.
 * Must be used when START command include url to pull image.
 */
void ota_firmware_pulled();

#ifdef __cplusplus
}
#endif

#endif // OTALIB_H_
