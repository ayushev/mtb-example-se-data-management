/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC 6 MCU and OPTIGA Trust M
*              for ModusToolbox.
*
* Related Document: See README.md
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "optiga_example.h"
#include "optiga/pal/pal_logger.h"
#include "optiga/pal/pal.h"
#include "provisioning_sm.h"

#define PROVISIONING_MODE

extern pal_logger_t logger_console;

#ifndef PROVISIONING_MODE

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

extern void example_optiga_crypt_hash (void);
extern void example_optiga_crypt_ecc_generate_keypair(void);
extern void example_optiga_crypt_ecdsa_sign(void);
extern void example_optiga_crypt_ecdsa_verify(void);
extern void example_optiga_crypt_ecdh(void);
extern void example_optiga_crypt_random(void);
extern void example_optiga_crypt_tls_prf_sha256(void);
extern void example_optiga_util_read_data(void);
extern void example_optiga_util_write_data(void);
extern void example_optiga_crypt_rsa_generate_keypair(void);
extern void example_optiga_crypt_rsa_sign(void);
extern void example_optiga_crypt_rsa_verify(void);
extern void example_optiga_crypt_rsa_decrypt_and_export(void);
extern void example_optiga_crypt_rsa_decrypt_and_store(void);
extern void example_optiga_crypt_rsa_encrypt_message(void);
extern void example_optiga_crypt_rsa_encrypt_session(void);
extern void example_optiga_util_update_count(void);
extern void example_optiga_util_protected_update(void);
extern void example_read_coprocessor_id(void);
extern void example_optiga_crypt_hash_data(void);
#ifdef OPTIGA_COMMS_SHIELDED_CONNECTION
extern void example_pair_host_and_optiga_using_pre_shared_secret(void);
#endif
extern void example_optiga_util_hibernate_restore(void);
extern void example_optiga_crypt_symmetric_encrypt_decrypt_ecb(void);
extern void example_optiga_crypt_symmetric_encrypt_decrypt_cbc(void);
extern void example_optiga_crypt_symmetric_encrypt_cbcmac(void);
extern void example_optiga_crypt_hmac(void);
extern void example_optiga_crypt_hkdf(void);
extern void example_optiga_crypt_symmetric_generate_key(void);
extern void example_optiga_hmac_verify_with_authorization_reference(void);
extern void example_optiga_crypt_clear_auto_state(void);

#endif

/*******************************************************************************
* Global Variables
*******************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU. It sets up a timer to trigger a 
* periodic interrupt. The main while loop checks for the status of a flag set 
* by the interrupt and toggles an LED at 1Hz to create an LED blinky. The 
* while loop also checks whether the 'Enter' key was pressed and 
* stops/restarts LED blinking.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();


    if(PAL_STATUS_SUCCESS != pal_logger_init(&logger_console))
	{
    	CY_ASSERT(0);
	}

    pal_init();

#ifndef PROVISIONING_MODE
    example_optiga_crypt_hash();
    example_optiga_crypt_ecc_generate_keypair();
    example_optiga_crypt_ecdsa_sign();
    example_optiga_crypt_ecdsa_verify();
    example_optiga_crypt_ecdh();
    example_optiga_crypt_random();
    example_optiga_crypt_tls_prf_sha256();
    example_optiga_util_read_data();
    example_optiga_util_write_data();
    example_optiga_crypt_rsa_generate_keypair();
    example_optiga_crypt_rsa_sign();
    example_optiga_crypt_rsa_verify();
    example_optiga_crypt_rsa_decrypt_and_export();
    example_optiga_crypt_rsa_decrypt_and_store();
    example_optiga_crypt_rsa_encrypt_message();
    example_optiga_crypt_rsa_encrypt_session();
    example_optiga_util_update_count();
    example_optiga_util_protected_update();
    example_read_coprocessor_id();
    example_optiga_crypt_hash_data();
    #ifdef OPTIGA_COMMS_SHIELDED_CONNECTION
    example_pair_host_and_optiga_using_pre_shared_secret();
    #endif
    example_optiga_util_hibernate_restore();
    example_optiga_crypt_symmetric_encrypt_decrypt_ecb();
    example_optiga_crypt_symmetric_encrypt_decrypt_cbc();
    example_optiga_crypt_symmetric_encrypt_cbcmac();
    example_optiga_crypt_hmac();
    example_optiga_crypt_hkdf();
    example_optiga_crypt_symmetric_generate_key();
    example_optiga_hmac_verify_with_authorization_reference();
    example_optiga_crypt_clear_auto_state();
#else
	while(1)
	{
		provisioning_state_machine(&logger_console);
	}
#endif
}

/* [] END OF FILE */
