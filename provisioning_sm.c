/******************************************************************************
* File Name:   provisioning_sm.c
*
* Description: This is the source file for the provisioning mode source code
*
*
* The MIT License
*
* Copyright (c) 2021 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*******************************************************************************/
#include "stdint.h"

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#include "optiga_example.h"
#include "optiga/pal/pal_logger.h"
#include "optiga/pal/pal.h"
#include "optiga/comms/optiga_comms.h"


#define PEER_DATA_BUFFER             (2000-2)
#define INIT_STATE                   (0x00U)
#define RX_STATE                     (0x01U)
#define TX_STATE                     (0x02U)
#define ERROR_STATE                  (0x03U)



/*******************************************************************************
* Function Prototypes
*******************************************************************************/


/*******************************************************************************
* Global Variables
*******************************************************************************/
uint8_t					state = INIT_STATE;
optiga_comms_t* 		p_comms;
uint8_t 				peer_data_buffer[PEER_DATA_BUFFER];
static volatile uint8_t optiga_comms_status = OPTIGA_COMMS_BUSY;
uint8_t 				chip_reponse_buffer[PEER_DATA_BUFFER];


/*******************************************************************************
* Function Name: optiga_comms_callback
********************************************************************************
* Summary: A cllaback which is given to the optiga stack worjing in a async mode
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
static void optiga_comms_callback(void * callback_ctx, optiga_lib_status_t event)
{
    optiga_comms_status = event;
}


/*******************************************************************************
* Function Name: comms_open
********************************************************************************
* Summary: A function to open a communication channel with the optiga
*
* Parameters:
*  pp_comms A double pointer to the comms instance.
*
* Return:
*  int
*
*******************************************************************************/
int comms_open(optiga_comms_t** pp_comms)
{
	// create comms
	*pp_comms = optiga_comms_create(optiga_comms_callback, NULL);
	(*pp_comms)->p_pal_os_event_ctx = (void*)pal_os_event_create(NULL, NULL);
	// perform comms open
	optiga_comms_status = OPTIGA_COMMS_BUSY;
	if(0 != optiga_comms_open(*pp_comms))
	{
		return 1;
	}
	//async wait
	while(0 != optiga_comms_status);
	return 0;

}


/*******************************************************************************
* Function Name: transceive_to_chip
********************************************************************************
* Summary: A function to synchronously transmit/receive data to the chip
*
* Parameters:
*  p_comms A pointer to the comms instance
*  p_tx_data buffer which contains data to be send to the chip
*  tx_data_length data length
*  p_rx_data buffer to store data from the chip
*  p_rx_data_len data length
*
* Return:
*  int
*
*******************************************************************************/
int transceive_to_chip( optiga_comms_t * p_comms,
						const uint8_t * p_tx_data,
						uint16_t tx_data_length,
						uint8_t * p_rx_data,
						uint16_t * p_rx_data_len)
{
	// transceive data to chip using comms tranceive
	optiga_comms_status = OPTIGA_COMMS_BUSY;
	if(OPTIGA_COMMS_SUCCESS != optiga_comms_transceive(p_comms, p_tx_data, tx_data_length, p_rx_data ,p_rx_data_len))
	{
		return 1;
	}
	//async wait
	while(OPTIGA_COMMS_SUCCESS != optiga_comms_status);

	return 0;
}


/*******************************************************************************
* Function Name: comms_close
********************************************************************************
* Summary: A function to close the communication channel between the endpoints
*
* Parameters:
*  p_comms A pointer to the comms instance.
*
* Return:
*  int
*
*******************************************************************************/
int comms_close( optiga_comms_t * p_comms)
{
    // close comms
    optiga_comms_status = OPTIGA_COMMS_BUSY;
    optiga_comms_close(p_comms);
    while(OPTIGA_COMMS_SUCCESS != optiga_comms_status);

    // destroy the comms
    optiga_comms_destroy(p_comms);
    return 1;
}


/*******************************************************************************
* Function Name: provisioning_state_machine
********************************************************************************
* Summary: A function to execute the state machine
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void provisioning_state_machine(pal_logger_t* p_logger_console)
{
	int error;
	static uint16_t buffer_length;
	uint16_t actual_apdu_length;

	switch(state)
	{
		case INIT_STATE:
		{
			// Open transparent channel
			error = comms_open(&p_comms);
			if(error)
			{
				state = ERROR_STATE;
				break;
			}
			state = RX_STATE;
		}
		break;
		case RX_STATE:
		{
			// Receive envelopedA APDU from peer
			error = pal_logger_read(p_logger_console, peer_data_buffer, PEER_DATA_BUFFER);
			if(error)
			{
				state = ERROR_STATE;
				break;
			}

			// Unpack data and send data to chip
			actual_apdu_length = (uint16_t)((peer_data_buffer[0] << 8) | (peer_data_buffer[1]));
			buffer_length = sizeof(chip_reponse_buffer);
			error = transceive_to_chip(p_comms, peer_data_buffer + 2, actual_apdu_length, chip_reponse_buffer, &buffer_length);
			if(error)
			{
				state = ERROR_STATE;
				break;

			}
			state = TX_STATE;
		}
		break;
		case TX_STATE:
		{
			// Send response from chip to peer
			// Pack data and send to peer
			peer_data_buffer[0] = (uint8_t)(buffer_length >> 8);
			peer_data_buffer[1] = (uint8_t)(buffer_length);
			memcpy(peer_data_buffer+2, chip_reponse_buffer,buffer_length);
			error = pal_logger_write(p_logger_console, peer_data_buffer, PEER_DATA_BUFFER);
			if(error)
			{
				state = ERROR_STATE;
				break;
			}
			state = RX_STATE;
		}
		break;
		case ERROR_STATE:
		{
			// Pack data and send to peer
			peer_data_buffer[0] = 0x00;
			peer_data_buffer[1] = 0x02;
			peer_data_buffer[2] = 0xFF;
			peer_data_buffer[3] = 0xFF;
			pal_logger_write(p_logger_console, peer_data_buffer, PEER_DATA_BUFFER);
			state = RX_STATE;
		}
		break;
		default:
			break;
	}
}

/* [] END OF FILE */
