/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
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

#include <stdio.h>
#include <time.h>
#include "fatfs.h"
#include "sqlite3.h"
#include "FreeRTOS.h"

uint8_t retSD;    /* Return value for SD */
FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */

/* USER CODE BEGIN Variables */
typedef enum {
  CARD_CONNECTED,
  CARD_DISCONNECTED,
  CARD_STATUS_CHANGED,
}SD_ConnectionStateTypeDef;

osMessageQId ConnectionEvent;

static void SD_Initialize(void);
static void FS_AppThread(void const *argument);
static void FS_FileOperations(void);
static void FATFS_SDDiskInit(char *path);

static uint8_t isInitialized = 0;
static uint8_t isFsCreated = 0;
static __IO uint8_t statusChanged = 0;
static void testdatabase(void);
uint8_t workBuffer[2 * _MAX_SS];
/*
 * ensure that the read buffer 'rtext' is 32-Byte aligned in address and size
 * to guarantee that any other data in the cache won't be affected when the 'rtext'
 * is being invalidated.
 */
ALIGN_32BYTES(uint8_t rtext[64]);

extern int sqlite_config_options();
/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  if(retSD == 0)
  {
	SD_Initialize();
	//osThreadDef(uSDThread, FS_AppThread, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	//osThreadCreate(osThread(uSDThread), NULL);
	FATFS_SDDiskInit(SDPath);
	/* Create Storage Message Queue */
	//osMessageQDef(osqueue, 10, uint16_t);
	//testdatabase();
	//ConnectionEvent = osMessageCreate (osMessageQ(osqueue), NULL);


  }
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
	    time_t rawtime;
	    time(&rawtime);
	    struct tm *ptm = localtime(&rawtime);
	    return (DWORD)(ptm->tm_year - 80) << 25
	           | (DWORD)(ptm->tm_mon + 1) << 21
	           | (DWORD)(ptm->tm_mday) << 16
	           | (DWORD)(ptm->tm_hour) << 11
	           | (DWORD)(ptm->tm_min) << 5
	           | (DWORD)(ptm->tm_sec / 2);
  /* USER CODE END get_fattime */  
}

/* USER CODE BEGIN Application */

/* USER CODE BEGIN Application */

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
	   configPRINTF("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   configPRINTF("\n");
   return 0;
}


static int Createtable()
{
	   sqlite3 *db;
	   char *zErrMsg = 0;
	   int rc;
	   char *sql;

	   /* Open database */
	   rc = sqlite3_open("test.db", &db);

	   if( rc ) {
		   configPRINTF("Can't open database: %s\n", sqlite3_errmsg(db));
	      return(rc);
	   } else {
		   configPRINTF("Opened database successfully\n");
	   }

	   /* Create SQL statement */
	   sql = "CREATE TABLE COMPANY("  \
	         "ID INT PRIMARY KEY     NOT NULL," \
	         "NAME           TEXT    NOT NULL," \
	         "AGE            INT     NOT NULL," \
	         "ADDRESS        CHAR(50)," \
	         "SALARY         REAL );";

	   /* Execute SQL statement */
	   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

	   if( rc != SQLITE_OK ){
		   configPRINTF("SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	   } else {
		   configPRINTF("Table created successfully\n");
	   }
	   sqlite3_close(db);
	   return 0;


}


static int InsertValues()
{
	   sqlite3 *db;
	   char *zErrMsg = 0;
	   int rc;
	   char *sql;

	   /* Open database */
	   rc = sqlite3_open("test.db", &db);

	   if( rc ) {
		   configPRINTF("Can't open database: %s\n", sqlite3_errmsg(db));
	      return(0);
	   } else {
		   configPRINTF("Opened database successfully\n");
	   }

	   /* Create SQL statement */
	   sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	         "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
	         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	         "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
	         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	         "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
	         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	         "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

	   /* Execute SQL statement */
	   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

	   if( rc != SQLITE_OK ){
		   configPRINTF("SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	   } else {
		   configPRINTF("Records created successfully\n");
	   }
	   sqlite3_close(db);
	   return 0;
}


static void testdatabase(void)
{
	  sqlite_config_options();
	  //sqlite3_os_init();

	  if(Createtable() != SQLITE_OK)
	  {
		  Error_Handler();
	  }
	  else
	  {
		  if(InsertValues() != SQLITE_OK)
		  {
			  Error_Handler();
		  }
	  }
}

static void SD_Initialize(void)
{
  if (isInitialized == 0)
  {
	BSP_SD_Init();
    BSP_SD_ITConfig();
     
    if(BSP_SD_IsDetected())
    {
      isInitialized = 1;
    }
  }
}

/**
  * @brief  Start task
  * @param  pvParameters not used
  * @retval None
  */
static void FS_AppThread(void const *argument)
{
  osEvent event;

    if(BSP_SD_IsDetected())
    {
      osMessagePut ( ConnectionEvent, CARD_CONNECTED, osWaitForever);
    }

  /* Infinite Loop */
  for( ;; )
  {
    event = osMessageGet( ConnectionEvent, osWaitForever );

    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
      case CARD_STATUS_CHANGED:
        if (BSP_SD_IsDetected())
        {
          osMessagePut ( ConnectionEvent, CARD_CONNECTED, osWaitForever);
        }
        else
        {
          //BSP_LED_Off(LED4);
          osMessagePut ( ConnectionEvent, CARD_DISCONNECTED, osWaitForever);

        }
        break;
      case CARD_CONNECTED:
        BSP_LED_Off(LED_RED);
        SD_Initialize();
        FS_FileOperations();
        statusChanged = 0;

        break;

      case CARD_DISCONNECTED:
        BSP_LED_Off(LED_GREEN);
        BSP_LED_On(LED_RED);

        if(isInitialized == 1)
        {
          isInitialized = 0;
          f_mount(NULL, (TCHAR const*)"", 0);
        }
        statusChanged = 0;

        break;
      }
    }
  }
}



/**
  * @brief  Main routine for Mass Storage Class
  * @param  None
  * @retval None
  */
#if 0
static void FS_FileOperations(void)
{
  FRESULT res;                                          /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs uSD + FreeRTOS CubeMX"; /* File write buffer */

  /* Register the file system object to the FatFs module */
  if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) == FR_OK)
  {
    /* check whether the FS has been already created */
    if (isFsCreated == 0)
    {
    	res = f_mkfs(SDPath, FM_FAT32, 0, workBuffer, sizeof(workBuffer));
      if(res != FR_OK)
      {
        BSP_LED_On(LED_RED);
        return;
      }
      isFsCreated = 1;
    }
    /* Create and Open a new text file object with write access */
    if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
      /* Write data to the text file */
      res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

      if((byteswritten > 0) && (res == FR_OK))
      {
        /* Close the open text file */
        f_close(&MyFile);

        /* Open the text file object with read access */
        if(f_open(&MyFile, "STM32.TXT", FA_READ) == FR_OK)
        {
          /* Read data from the text file */
          res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

          if((bytesread > 0) && (res == FR_OK))
          {
            /* Close the open text file */
            f_close(&MyFile);

            /* Compare read data with the expected data */
            if((bytesread == byteswritten))
            {
              /* Success of the demo: no error occurrence */
              BSP_LED_On(LED_GREEN);
              return;
            }
          }
        }
      }
    }
  }
  /* Error */
  BSP_LED_On(LED_RED);
}
#endif
//#if 0
static void FS_FileOperations(void)
{
  FILE *fil;
  FRESULT res;                                          /* FatFs function common result code */
  size_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs uSD + FreeRTOS CubeMX"; /* File write buffer */

  /* Register the file system object to the FatFs module */
  if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) == FR_OK)
  {
    /* check whether the FS has been already created */
    if (isFsCreated == 0)
    {
      if(f_mkfs(SDPath, FM_FAT32, 0, workBuffer, sizeof(workBuffer)) != FR_OK)
      {
        BSP_LED_On(LED_RED);
        return;
      }
      isFsCreated = 1;
    }
    fil = fopen("STM321.txt","w+");
    if(fil)
    {
    	vLoggingPrintf("The file is now opened\n");
    	byteswritten = fwrite(wtext, sizeof(uint8_t), sizeof(wtext),fil);
    	if(byteswritten > 0)
    	{
    		vLoggingPrintf("The file is now closed with %d bytes written\n",byteswritten);
			fclose(fil);

			fil = fopen("STM321.txt","r");
			if(fil)
			{
				bytesread = fread(rtext, sizeof(uint8_t), sizeof(rtext),fil);
				vLoggingPrintf("The file is now opened with %d bytes read\n",bytesread);

				if(bytesread > 0)
				{
					fclose(fil);
		            /* Compare read data with the expected data */
		            if((bytesread == byteswritten))
		            {
		              /* Success of the demo: no error occurrence */
		              vLoggingPrintf("The read data length and write data length matches\n");
		              BSP_LED_On(LED_GREEN);
		              return;
		            }
				}
			}

    	}
    }
  }
	fclose(fil);
  /* Error */
  BSP_LED_On(LED_RED);
}
//#endif
/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(MFX_IRQOUT_PIN == GPIO_Pin)
  {
    if (statusChanged == 0)
    {
      statusChanged = 1;
      osMessagePut ( ConnectionEvent, CARD_STATUS_CHANGED, osWaitForever);
    }
  }
}


static void FATFS_SDDiskInit( char *path )
{
	  /* Register the file system object to the FatFs module */
	  if(f_mount(&SDFatFs, (TCHAR const*)path, 1) == FR_OK)
	  {
		  if (SDFatFs.fs_type == FS_FAT32)
			  isFsCreated = 1;
	  }

	    /* check whether the FS has been already created */
	    if (isFsCreated == 0)
	    {
	      if(f_mkfs(SDPath, FM_FAT32, 0, workBuffer, sizeof(workBuffer)) != FR_OK)
	      {
	        BSP_LED_On(LED_RED);
	        return;
	      }
	      isFsCreated = 1;
	    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
