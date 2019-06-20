/** 
  *
  *  Portions COPYRIGHT 2017 STMicroelectronics
  *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
  *
  ******************************************************************************
  * @file    mbedTLS/SSL_Server/Src/ssl_server.c
  * @author  MCD Application Team
  * @brief   SSL server application 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "main.h"
#include "cmsis_os.h"
#include <lwip.h>
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_time       time
#define mbedtls_time_t     time_t 
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#include <stdlib.h>
#include <string.h>

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif



const char server_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDRDCCAiygAwIBAgIBATANBgkqhkiG9w0BAQsFADAVMRMwEQYDVQQDEwoxMC4x\r\n"
"MC45Ljc3MB4XDTE5MDMyNjE4MTExNFoXDTI5MDMyNjE4MTExNFowFTETMBEGA1UE\r\n"
"AxMKMTAuMTAuOS43NzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAI73\r\n"
"P7SM+JPr4XW+hXihbwslZJni86mERSXFhyQRl21yXe5Ak3CsVTOU5YHe1uhX5mMS\r\n"
"4fKAFFgeIWDHdBjkxhw6pPsTlZAuOW0SI4pRasxvtL3yTuWpF1SIspgv7G1rSk5Y\r\n"
"rcU8sLAHZhQn6aAT7zTJ54o2xMYMjOvszoSYRv5J+W13U82Gu1qhontz7oX6RXL6\r\n"
"LICbg1QgbU7Atqrx3zn9XCgjXt74dndFqTbL8ErtLiRM7mNcKev79TTvY+gr8fgC\r\n"
"j6FEdBkbsLOqPqgejhKnn7PbzO65eFQ9lhEaoyTkzPaAWJZs5badMZGCvsUFrxp1\r\n"
"G/+jyVWDhI0awod7I4kCAwEAAaOBnjCBmzAMBgNVHRMEBTADAQH/MAsGA1UdDwQE\r\n"
"AwIC9DA7BgNVHSUENDAyBggrBgEFBQcDAQYIKwYBBQUHAwIGCCsGAQUFBwMDBggr\r\n"
"BgEFBQcDBAYIKwYBBQUHAwgwEQYJYIZIAYb4QgEBBAQDAgD3MA8GA1UdEQQIMAaH\r\n"
"BAoKCU0wHQYDVR0OBBYEFEDfQyOoKFLCtXrj8Ea46wAR9yeGMA0GCSqGSIb3DQEB\r\n"
"CwUAA4IBAQAB4e1cgNjc0Yh062EKOquCnYTXqv4OobrfEtuacfQjB7fYJLsDlF+Y\r\n"
"c3MPWRyRHMSd/D5wliSnsBIYh5pi/8jLIX8MIUTgmYEmxmcGM3EzhzEGrVJu40ZD\r\n"
"/UrhqmMo+wgWLFQYSvSKqK/1SINMjt9Rx71LD1UsTLUxc09oeNtelIo/GBaCMnqg\r\n"
"KOOsqQU1rooB6hNbcU8wSWCPN6fCh5XgECsB9viI+3JR03u6RUYWIKREQYxSxHGs\r\n"
"pdD/NWiAeJ6eXYZK4wc9TO02aQcUrxPyRMc+gDatCafin/Mr28U22vts0eFh6yEw\r\n"
"MWZIicNXdz17YZr7FV8Onj0ervP7idbh\r\n"
"-----END CERTIFICATE-----\r\n";

const char server_key[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEowIBAAKCAQEAjvc/tIz4k+vhdb6FeKFvCyVkmeLzqYRFJcWHJBGXbXJd7kCT\r\n"
"cKxVM5Tlgd7W6FfmYxLh8oAUWB4hYMd0GOTGHDqk+xOVkC45bRIjilFqzG+0vfJO\r\n"
"5akXVIiymC/sbWtKTlitxTywsAdmFCfpoBPvNMnnijbExgyM6+zOhJhG/kn5bXdT\r\n"
"zYa7WqGie3PuhfpFcvosgJuDVCBtTsC2qvHfOf1cKCNe3vh2d0WpNsvwSu0uJEzu\r\n"
"Y1wp6/v1NO9j6Cvx+AKPoUR0GRuws6o+qB6OEqefs9vM7rl4VD2WERqjJOTM9oBY\r\n"
"lmzltp0xkYK+xQWvGnUb/6PJVYOEjRrCh3sjiQIDAQABAoIBACswOgh7BaT5/xQo\r\n"
"x/1XF0qQs0FhtCYjlRgCAf89aTdg80tEKsZA6UQ52NNQ1C9ATpLjY2p0krDD+WVZ\r\n"
"G5NhmQ9yD+DhW69Ap4xxjFb+nXrAzCMNjhQG7stUuE8NOyka3kmotGX/HYkv0wCH\r\n"
"xiXbn8mcrNuuC+egZvnmgcTAk8HBdywFPUVzOFUgXwZbTYew7KT66wjg+ZnwpomQ\r\n"
"3r1WgSR0GVBV5m+u8+nTsR6pDFcGNDd3vjwqONs9o5oYQr78cNrJsEU7fJMbOdFA\r\n"
"AKabKP1CEijwrY57MiocTJjInHA93yAaFesgX2e9extNbtcANOhGgD2AZXg5I36O\r\n"
"nlL+9wECgYEAzwHf/qw8qzZ+RQHzZndwFLw/7ok6mZaJhjc4iK+cpXNiCXL9HXD/\r\n"
"6zS+Tu8fd57ETzFyvlYDA0bgqhR7f6jzNlzaY4TylL+voasHAM3fzPeHs5ZoMJ0a\r\n"
"6xtW0+KG3iAk+S57s0qRhu4q0tlO3zfuEG608wVCUK9uW5y+7gLCEpkCgYEAsM08\r\n"
"2nYiMBnjVLLBKPpp9oBfeBQ9UmOW2EVJvynxzxyhgcT19nadzWB7KOgJbfBtGkCr\r\n"
"TiFBZaz2RnMHfxZy8s3JP36fEhTQnviUhyKiEQmaW1S7ZRcV8ceSVqMXmBzL1cXh\r\n"
"wjglnGIGY3fGgSBVF9MKm5hH7wiHQ197miebHnECgYEAqE7I9YLKrRZYYfktEkHI\r\n"
"EkPg4ywWRiw8UrDxOUk3XtpKeo5QuDBZE77FgN4xQowG5rC4r4K1XmOxZPfUVb5j\r\n"
"SO8D7Wh553FW9mZdAk1hSoBziBOhUO1bpo6K8Sm8KDwQc71UfDkDy6Y3kUl4NEuA\r\n"
"N0Ibw3+iJIIOg1QVdOUvL/kCgYBXE96C4mgYNMfocVn9ashBphxfMei56V7kCcw2\r\n"
"oNv92+YX2gAicW1rN7exAAeU5DN4GJ3Gj058W2G4dPMePtpCh+Q0p5KE/0d45ImS\r\n"
"vRYBOi+SB/t74JDoUhjbHtyCCLzD/wAf/jNYAFeRKe7Uri362/B/jdLwAc1M+3wP\r\n"
"Aq69oQKBgGM3bad8W3lNtzLGYoh9HdcKOGW5kulR4eCl9NMI49y3Yco48KY77y6s\r\n"
"acvRELe5FcEVyRanlulDXXW+STmCmdKHMDJIGgtgWf9qTY+56USxfOHKfZ3qHD0r\r\n"
"C9DlrxcMkXOCLinJfYsk0WC9MOLKI+/mP1G976u6IDXThYZ/7JXc\r\n"
"-----END RSA PRIVATE KEY-----\r\n";



static mbedtls_net_context listen_fd, client_fd;
static uint8_t buf[1024];
static const char *pers = "ssl_server";
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt srvcert;
mbedtls_pk_context pkey;

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_context cache;
#endif
  /**
   * Debug callback for mbed TLS
   * Just prints on the USB serial port
   */
  static void my_debug(void *ctx, int level, const char *file, int line,
                       const char *str)
  {
      const char *p, *basename;
      (void) ctx;

      /* Extract basename from file */
      for(p = basename = file; *p != '\0'; p++) {
          if(*p == '/' || *p == '\\') {
              basename = p + 1;
          }
      }

      mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
  }


void SSL_Server(void const *argument)
{
  int ret, len;
  UNUSED(argument);
  
  
#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
  mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
#endif
  mbedtls_net_init( &listen_fd );
  mbedtls_net_init( &client_fd );
  mbedtls_ssl_init( &ssl );
  mbedtls_ssl_config_init( &conf );
  mbedtls_ssl_conf_dbg(&conf, my_debug, NULL);
  mbedtls_debug_set_threshold(0);
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_init( &cache );
#endif
  mbedtls_x509_crt_init( &srvcert );
  mbedtls_pk_init( &pkey );
  mbedtls_entropy_init( &entropy );
  mbedtls_ctr_drbg_init( &ctr_drbg );

  /*
   * 1. Load the certificates and private RSA key
   */
  mbedtls_printf( "\n  . Loading the server cert. and key..." );

  /*
   * This demonstration program uses embedded test certificates.
   * Using mbedtls_x509_crt_parse_file() to read the server and CA certificates
   * resuires the implmentation of the File I/O API using the FatFs, that is 
   * not implemented yet.
   */
#if defined(MBEDTLS_FS_IO)
  //ret = mbedtls_x509_crt_parse_file(&srvcert, "certs/servercert.crt");
  ret = mbedtls_x509_crt_parse_file(&srvcert, "1010987.cert");
#else
  ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) server_cert, sizeof(server_cert) );
#endif
  //ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len );

  if(ret != 0)
  {
    mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
    goto exit;
  }
/*
  ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
    goto exit;
  }
*/
#if defined(MBEDTLS_FS_IO)
  //ret = mbedtls_pk_parse_keyfile(&pkey, "certs/serverkey.key", NULL);
  ret = mbedtls_pk_parse_keyfile(&pkey, "1010987.key", NULL);
#else
  ret =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) server_key, sizeof(server_key), NULL, 0);
#endif
  //ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0 );
  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 2. Setup the listening TCP socket
   */

  mbedtls_printf( "  . Bind on https://%s:%s/ ...", get_current_ipaddress(),SERVER_PORT );

  if((ret = mbedtls_net_bind(&listen_fd, get_current_ipaddress(), SERVER_PORT , MBEDTLS_NET_PROTO_TCP )) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 3. Seed the RNG
   */
  mbedtls_printf( "  . Seeding the random number generator..." );

  if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen( (char *)pers))) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 4. Setup stuff
   */
  mbedtls_printf( "  . Setting up the SSL data...." );

  if((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

  mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
  if((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
    goto exit;
  }

  if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

reset:
#ifdef MBEDTLS_ERROR_C
  if(ret != 0)
  {
    uint8_t error_buf[100];
    mbedtls_strerror( ret, (char *)error_buf, 100 );
    mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
  }
#endif
  
  mbedtls_net_free(&client_fd);

  mbedtls_ssl_session_reset(&ssl);

  /*
   * 5. Wait until a client connects
   */
  mbedtls_printf( "  . Waiting for a remote connection ...\n" );

  if((ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)
  {
    mbedtls_printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

  mbedtls_printf(" ok\n");

  /*
   * 6. Handshake
   */
  mbedtls_printf("  . Performing the SSL/TLS handshake...");

  while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret);
      goto reset;
    }
  }

  mbedtls_printf(" ok\n");

  /*
   * 7. Read the HTTP Request
   */
  mbedtls_printf("  < Read from client:");
  do
  {
    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_RED);
    ret = mbedtls_ssl_read(&ssl, buf, len);

    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      continue;
    }
    if(ret <= 0)
    {
      switch(ret)
      {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          mbedtls_printf(" connection was closed gracefully\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        case MBEDTLS_ERR_NET_CONN_RESET:
          mbedtls_printf(" connection was reset by peer\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        default:
          mbedtls_printf(" mbedtls_ssl_read returned -0x%x\n", -ret);
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;
      }
      
      HAL_Delay(100);
      BSP_LED_On(LED_GREEN);
      break;
    }

    len = ret;
    mbedtls_printf(" %d bytes read\n\n%s", len, (char *) buf);

    if(ret > 0)
    {
      break;
    }
  } while(1);

  /*
   * 8. Write the 200 Response
   */
  mbedtls_printf( "  > Write to client:" );
  len = sprintf((char *) buf, HTTP_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));

  while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
  {
    if(ret == MBEDTLS_ERR_NET_CONN_RESET)
    {
      mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
      goto reset;
    }

    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
      goto exit;
    }
  }

  len = ret;
  mbedtls_printf(" %d bytes written\n\n%s\n", len, (char *) buf);

  mbedtls_printf("  . Closing the connection...");

  while((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret );
      goto reset;
    }
  }

  if (ret == 0)
  {
    BSP_LED_On(LED_GREEN);
    mbedtls_printf( " ok\n" );
  }
  
  ret = 0;
  goto reset;

exit:
  BSP_LED_Off(LED_GREEN);
  BSP_LED_On(LED_RED);

  mbedtls_net_free( &client_fd );
  mbedtls_net_free( &listen_fd );

  mbedtls_x509_crt_free( &srvcert );
  mbedtls_pk_free( &pkey );
  mbedtls_ssl_free( &ssl );
  mbedtls_ssl_config_free( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_free( &cache );
#endif
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );

}
