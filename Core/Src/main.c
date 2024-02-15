/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hdc1080.h"
#include "LDC_1614.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define Base_Can_address 0x10

union Candata {
 struct { uint32_t data0 :32;
		 uint32_t data1 :32;
 } u32data ;
   struct { int data_i0 :16;
		  int data_i1 :16;
		  int data_i2 :16;
		  int data_i3 :16;
 } i16data ;
   struct { uint16_t data_u0 :16;
   uint16_t  data_u1 :16;
   uint16_t  data_u2 :16;
   uint16_t  data_u3 :16;
 } u16data ;
 unsigned char stmp[8];
 long dataL;
} mycandata,mycandata1,mycandata2,mycandata3,mycandata4,mycandata5,mycandata6,mycandata7,mycandata8,MyRxcan;//[CHANNEL_NUM];

//long sample_Rate_millis=5000.0/SAMPLE_LENGTH;
volatile uint32_t Last_Time ; //10khz clock. 16mhz/1600 prescaler.
volatile int idx[CHANNEL_NUM];

uint32_t sen0_ch0;
uint32_t sen0_ch1;
//uint32_t sen0_ch2;
//uint32_t sen0_ch3;
uint8_t drivecurrent[CHANNEL_NUM]={Base_Drive_Current,Base_Drive_Current};//Base_Drive_Current,Base_Drive_Current
volatile uint16_t status0=0,status1=0;//,status2=0,status3=0
volatile uint8_t status0C=0,status1C=0;//,status2C=0,status3C=0
volatile uint16_t N_AE_0C=0,N_AE_1C=0;//,N_AE_2C=0,N_AE_3C=0
//volatile uint16_t AEL[4]={0,0,0,0};
volatile uint8_t AEH[2]={0,0};

uint8_t index_data=0;

volatile uint8_t flag0=0;//,flag1=0,flag2=0,flag3=0;
volatile uint8_t flag_sample=0;
 uint32_t Data0[2*SAMPLE_LENGTH]; // use two buffers for calculating stats while collecting data.
 uint32_t Data1[2*SAMPLE_LENGTH];
// uint32_t Data2[2*SAMPLE_LENGTH];
// uint32_t Data3[2*SAMPLE_LENGTH];
uint8_t datahalf=0;
volatile uint32_t sensor_status=0;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	//if(GPIO_Pin==GPIO_PIN_4){
	if (htim->Instance == TIM3) {


	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);

	//}
	uint8_t index=0;
	sensor_status=LDC1614_get_sensor_status();
	index=(0x3 & (sensor_status>>14));

	// if data ready, read the data, else parse the error and decide what to do
	if(TestBitu32(sensor_status,6)  )
			{
			//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
			flag_sample=1;

			status0=LDC1614_get_channel_result(0, &Data0[index_data+datahalf*SAMPLE_LENGTH]);
			//=sen0_ch0;
			status1=LDC1614_get_channel_result(1, &Data1[index_data+datahalf*SAMPLE_LENGTH]);
			//=sen0_ch1;
//			status2=LDC1614_get_channel_result(2, &Data2[index_data+datahalf*SAMPLE_LENGTH]);
//			//=sen0_ch2;
//			status3=LDC1614_get_channel_result(3, &Data3[index_data+datahalf*SAMPLE_LENGTH]);
			//=sen0_ch3;




			index_data++;
	//			index_data1++;
	//			index_data2++;
	//			index_data3++;

			if(index_data>=SAMPLE_LENGTH){

				index_data=0;

				if(datahalf==0){
					datahalf=1;
				}
				else{
					datahalf=0;
				}

				flag0=1;

			}






			// HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //blue LED
			status0C=status0C|(0xFF & status0);
			status1C=status1C|(0xFF & status1);
//			status2C=status2C|(0xFF & status2);
//			status3C=status3C|(0xFF & status3);
				if(TestBitu32(status0,0) ){
					N_AE_0C++;
				}
				if(TestBitu32(status1,0) ){
					N_AE_1C++;
				}
//				if(TestBitu32(status2,0) ){
//					N_AE_2C++;
//				}
//				if(TestBitu32(status3,0) ){
//					N_AE_3C++;
//				}

			 // reset the counter


//			if(index_data1>SAMPLE_LENGTH){
//				index_data1=0;
//
//				flag1=1;
//			}
//			if(index_data2>SAMPLE_LENGTH){
//				index_data2=0;
//
//				flag2=1;
//			}
//			if(index_data3>SAMPLE_LENGTH){
//				index_data3=0;
//
//				flag3=1;
//			}

	}
	else{

		//if( (sensor_status & ((u16)1<<0) )  | (sensor_status & ((u16)1<<1)) | (sensor_status & ((u16)1<<2)) | (sensor_status & ((u16)1<<3)) ){








			switch(index){
				case 0 :
					status0=LDC1614_get_channel_result(0, &sen0_ch0); // clear junk from the registers.
					if(TestBitu32(status0,0) ){
						N_AE_0C++;
					}
					break;
				case 1 :
					status1=LDC1614_get_channel_result(1, &sen0_ch1);
					if(TestBitu32(status1,0) ){
						N_AE_1C++;
					}
					break;
//				case 2 :
//					status2=LDC1614_get_channel_result(2, &sen0_ch2);
//					if(TestBitu32(status2,0) ){
//						N_AE_2C++;
//					}
//					break;
//				case 3 :
//					status3=LDC1614_get_channel_result(3, &sen0_ch3);
//					if(TestBitu32(status3,0) ){
//						N_AE_3C++;
//					}
//					break;

			}
			//flag0=2; //set a flag to send status via can
		}

	//	AEL[index]+= (sensor_status & ((u16)1<<9))>>9;
		AEH[index]+= (sensor_status & ((u16)1<<10))>>10;
//		if(AEL[index]>25000)
//		{
//			AEL[index]=25000;
//			flag0=2; // racking up the errors. so get ready to increment or decrement the drive current.
//			}
		if(AEH[index]>250){
			//AEH[index]=0;
			flag0=2;
		}

		if((N_AE_1C>250)|(N_AE_0C>250)){
			flag0=2;
		}
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAN_RxHeaderTypeDef rxHeader; //CAN Bus Transmit Header
CAN_TxHeaderTypeDef txHeader,txHeader1,txHeader2,txHeader3,txHeader4,txHeader5,txHeader6,txHeader7,txHeader8; //CAN Bus Receive Header
uint8_t canRX[8] = {0,0,0,0,0,0,0,0};  //CAN Bus Receive Buffer

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int valid_samples=0;
	u32 Data_quality[(SAMPLE_LENGTH/32)+1];
	uint16_t T_raw;
	uint32_t mean0,std0,mean1,std1;

	uint16_t RH_raw;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  CAN_FilterTypeDef canfil; //CAN Bus Filter
   uint32_t canMailbox; //CAN Bus Mail box variable

   // Accept all messages
   canfil.FilterBank = 0;
   canfil.FilterMode = CAN_FILTERMODE_IDMASK;
   canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
   canfil.FilterIdHigh = (Base_Can_address+0xA)<<5;//0;
   canfil.FilterIdLow = 0;
   canfil.FilterMaskIdHigh = (Base_Can_address+0xA)<<5;//0;
   canfil.FilterMaskIdLow = 0;
   canfil.FilterScale = CAN_FILTERSCALE_32BIT;
   canfil.FilterActivation = ENABLE;
   canfil.SlaveStartFilterBank = 14;

   txHeader.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader.IDE = CAN_ID_STD;
   txHeader.RTR = CAN_RTR_DATA;
   txHeader.StdId = Base_Can_address;
   txHeader.ExtId = 0x02;
   txHeader.TransmitGlobalTime = DISABLE;

   txHeader1.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader1.IDE = CAN_ID_STD;
   txHeader1.RTR = CAN_RTR_DATA;
   txHeader1.StdId = Base_Can_address+1;
   txHeader1.ExtId = 0x02;
   txHeader1.TransmitGlobalTime = DISABLE;

   txHeader2.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader2.IDE = CAN_ID_STD;
   txHeader2.RTR = CAN_RTR_DATA;
   txHeader2.StdId = Base_Can_address+2;
   txHeader2.ExtId = 0x02;
   txHeader2.TransmitGlobalTime = DISABLE;


   txHeader3.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader3.IDE = CAN_ID_STD;
   txHeader3.RTR = CAN_RTR_DATA;
   txHeader3.StdId = Base_Can_address+3;
   txHeader3.ExtId = 0x02;
   txHeader3.TransmitGlobalTime = DISABLE;


   txHeader4.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader4.IDE = CAN_ID_STD;
   txHeader4.RTR = CAN_RTR_DATA;
   txHeader4.StdId = Base_Can_address+4;
   txHeader4.ExtId = 0x02;
   txHeader4.TransmitGlobalTime = DISABLE;

   txHeader5.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader5.IDE = CAN_ID_STD;
   txHeader5.RTR = CAN_RTR_DATA;
   txHeader5.StdId = Base_Can_address+5;
   txHeader5.ExtId = 0x02;
   txHeader5.TransmitGlobalTime = DISABLE;

   txHeader6.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader6.IDE = CAN_ID_STD;
   txHeader6.RTR = CAN_RTR_DATA;
   txHeader6.StdId = Base_Can_address+6;
   txHeader6.ExtId = 0x02;
   txHeader6.TransmitGlobalTime = DISABLE;

   txHeader7.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader7.IDE = CAN_ID_STD;
   txHeader7.RTR = CAN_RTR_DATA;
   txHeader7.StdId = Base_Can_address+7;
   txHeader7.ExtId = 0x02;
   txHeader7.TransmitGlobalTime = DISABLE;

   txHeader8.DLC = 8; // Number of bites to be transmitted max- 8
   txHeader8.IDE = CAN_ID_STD;
   txHeader8.RTR = CAN_RTR_DATA;
   txHeader8.StdId = Base_Can_address+8;
   txHeader8.ExtId = 0x02;
   txHeader8.TransmitGlobalTime = DISABLE;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); //blue LED
  HAL_Delay(100);

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
//  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);

  LDC1614_reset_sensor();
  HAL_CAN_ConfigFilter(&hcan,&canfil); //Initialize CAN Filter
  HAL_CAN_Start(&hcan); //Initialize CAN Bus
  HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);// Initialize CAN Bus Rx Interrupt

  LDC1614_mutiple_channel_config();


    hdc1080_measureRH(&RH_raw);
    hdc1080_measureT(& T_raw);
    // READ_MANUFACTURER_ID
  // reading the Manuf ID Works.
     IIC_read_16bit(READ_MANUFACTURER_ID, &T_raw);
 //   while (1)
 // {
 //   T_raw+=10;
     mycandata.stmp[0]=T_raw;
     mycandata.stmp[1]=T_raw>>8;
     mycandata.stmp[2]=0;
     mycandata.stmp[3]=0;
     mycandata.stmp[4]=1;
     mycandata.stmp[5]=0;
     mycandata.stmp[6]=0;
     mycandata.stmp[7]=0;

     //txHeader.StdId = Base_Can_address;
     while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
     RH_raw=HAL_CAN_AddTxMessage(&hcan,&txHeader,mycandata.stmp,&canMailbox); // Send Message

//	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
     HAL_Delay(100);
  //}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if(flag_sample){
			  Last_Time=__HAL_TIM_GET_COUNTER(&htim2); // reset the counter either way to prevent rapid interrupt firing from the software timer.
			  flag_sample=0;
			 // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); //blue LED
		  }

	  // this code should no longer be needed siegeljb Feb 2 2024
		  if(__HAL_TIM_GET_COUNTER(&htim2)-Last_Time> 110000){ // should be 100ms  //20ms per tick
	//Last_Time=__HAL_TIM_GET_COUNTER(&htim2)
			  //HAL_GPIO_EXTI_Callback(9);

			  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); //blue LED

				sensor_status=LDC1614_get_sensor_status();
				status0=LDC1614_get_channel_result(0, &sen0_ch0); // clear junk from the registers.
				status1=LDC1614_get_channel_result(1, &sen0_ch1);

				//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				Last_Time=__HAL_TIM_GET_COUNTER(&htim2);

		  }


		  if(flag0==2){ // we got more than 250 errors with no data ready, so report them back.


				  mycandata5.u32data.data0=sensor_status;
				  mycandata5.stmp[4]=drivecurrent[0];
				  mycandata5.stmp[5]=drivecurrent[1];
	//			  mycandata5.stmp[6]=drivecurrent[2];
	//			  mycandata5.stmp[7]=drivecurrent[3];
				   //txHeader.StdId = Base_Can_address+5;
				  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
				  HAL_CAN_AddTxMessage(&hcan,&txHeader5,mycandata5.stmp,&canMailbox);

				  mycandata6.u16data.data_u0 =status0;
				  mycandata6.u16data.data_u1 =status1;
	//			  mycandata6.u16data.data_u2 =status2;
	//			  mycandata6.u16data.data_u3 =status3;
				  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
				  HAL_CAN_AddTxMessage(&hcan,&txHeader6,mycandata6.stmp,&canMailbox);

				//  mycandata7.u16data.data_u0 =AEL[0];
				//  mycandata7.u16data.data_u1 =AEL[1];
				//  mycandata7.u16data.data_u2 =AEL[2];
				//  mycandata7.u16data.data_u3 =AEL[3];
				//  HAL_CAN_AddTxMessage(&hcan,&txHeader7,mycandata7.stmp,&canMailbox);

				  mycandata8.u16data.data_u0 =AEH[0];
				  mycandata8.u16data.data_u1 =AEH[1];
	//			  mycandata8.u16data.data_u2 =AEH[2];
	//			  mycandata8.u16data.data_u3 =AEH[3];
				  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
				  HAL_CAN_AddTxMessage(&hcan,&txHeader8,mycandata8.stmp,&canMailbox);

				  AEH[0]=0;
				  AEH[1]=0;
				  //AEH[2]=0;
				  //AEH[3]=0;

				  mycandata4.stmp[0]=T_raw;
				  mycandata4.stmp[1]=T_raw>>8;
				  mycandata4.stmp[2]=RH_raw;
				  mycandata4.stmp[3]=RH_raw>>8;
				  mycandata4.stmp[4]=N_AE_0C;
				  mycandata4.stmp[5]=N_AE_1C;
	//			  mycandata4.stmp[6]=N_AE_2C;
	//			  mycandata4.stmp[7]=N_AE_3C;
					N_AE_0C=0;
					N_AE_1C=0;
	//				N_AE_2C=0;
	//				N_AE_3C=0;

				 // csend[6]=receive_data_RHT[2];
				 // csend[7]=receive_data_RHT[3]; // Tx Buffer
				//  txHeader.StdId = Base_Can_address+4;
				  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
				  HAL_CAN_AddTxMessage(&hcan,&txHeader4,mycandata4.stmp,&canMailbox); // Send Message

	//		  for(i=0;i<CHANNEL_NUM;i++){
	////				  if(AEL[i]>5)
	////					AEL[i]-= 5;
	////				  else
	////					AEL[i]=0;
	//
	//				  if(AEL[i]>100){
	//					  drivecurrent[i]++;
	//					  if(drivecurrent[i] >Max_Drive_Current)
	//						  drivecurrent[i] =Max_Drive_Current;
	//					  LDC1614_set_sensor_config(0xC601); //enter sleep
	//					  LDC1614_set_driver_current(i, drive_current_to_hex(drivecurrent[i]) );
	//					  LDC1614_set_sensor_config(0xD601); //resume
	//					 AEL[i]=0;
	//				  }
	//
	////				  if(AEH[i]>5)
	////					AEH[i]-= 5;
	////				  else
	////					AEH[i]=0;
	//
	//				  if(AEH[i]>100){
	//					  drivecurrent[i]--;
	//					  if(drivecurrent[i] <Min_Drive_Current)
	//						  drivecurrent[i] =Min_Drive_Current;
	//					  LDC1614_set_sensor_config(0xC601);
	//					  LDC1614_set_driver_current(i, drive_current_to_hex(drivecurrent[i]) );
	//					  LDC1614_set_sensor_config(0xD601);
	//					  AEH[i]=0;
	//				  }
	//		  }
			  flag0=0;
			  Last_Time=__HAL_TIM_GET_COUNTER(&htim2);
		  }
		  if(flag0==1){

			  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
				//Tten_conv=((T_raw*1650)>>16)-400;
				//T=( ((T_raw/65536.0)*165.0)-40.0 );
			 //     sprintf(Buffer, "T: %u raw, RH: %d , T: %ld C \r\n",T_raw,RH_raw*100>>16,Tten_conv);
			 //   HAL_UART_Transmit(&huart2, Buffer, sizeof(Buffer), 10000);
			   mycandata5.u32data.data0=sensor_status;
			   mycandata5.stmp[4]=drivecurrent[0];
			   mycandata5.stmp[5]=drivecurrent[1];
	//		   mycandata5.stmp[6]=drivecurrent[2];
	//		   mycandata5.stmp[7]=drivecurrent[3];
			   //txHeader.StdId = Base_Can_address+5;
			   while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			   HAL_CAN_AddTxMessage(&hcan,&txHeader5,mycandata5.stmp,&canMailbox);


			  flag0=0;
			  get_stats(&Data0[(1-datahalf)*SAMPLE_LENGTH],&std0,&mean0);
			  get_valid_stats(&Data0[(1-datahalf)*SAMPLE_LENGTH],Data_quality, &std0,&mean0,&valid_samples);


			  mycandata.u32data.data0 =mean0;
			  mycandata.u16data.data_u2 =std0;
			  mycandata.u16data.data_u3 =valid_samples;
			  //txHeader.StdId = Base_Can_address;
			  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			  HAL_CAN_AddTxMessage(&hcan,&txHeader,mycandata.stmp,&canMailbox);


			  hdc1080_measureRH(&RH_raw);
			  hdc1080_measureT(& T_raw);


			  mycandata4.stmp[0]=T_raw;
			  mycandata4.stmp[1]=T_raw>>8;
			  mycandata4.stmp[2]=RH_raw;
			  mycandata4.stmp[3]=RH_raw>>8;
			  mycandata4.stmp[4]=N_AE_0C;
			  mycandata4.stmp[5]=N_AE_1C;
	//		  mycandata4.stmp[6]=N_AE_2C;
	//		  mycandata4.stmp[7]=N_AE_3C;
				N_AE_0C=0;
				N_AE_1C=0;
	//			N_AE_2C=0;
	//			N_AE_3C=0;

			 // csend[6]=receive_data_RHT[2];
			 // csend[7]=receive_data_RHT[3]; // Tx Buffer
			//  txHeader.StdId = Base_Can_address+4;
			  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			  HAL_CAN_AddTxMessage(&hcan,&txHeader4,mycandata4.stmp,&canMailbox); // Send Message

			  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); // Green LED

			  mycandata6.u16data.data_u0 =status0C;
			  mycandata6.u16data.data_u1 =status1C;
	//		  mycandata6.u16data.data_u2 =status2C;
	//		  mycandata6.u16data.data_u3 =status3C;
			  status0C=0;
			  status1C=0;
	//		  status2C=0;
	//		  status3C=0;

	//		  txHeader.StdId = Base_Can_address+6;
			  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			  HAL_CAN_AddTxMessage(&hcan,&txHeader6,mycandata6.stmp,&canMailbox);

			  get_stats(&Data1[(1-datahalf)*SAMPLE_LENGTH],&std1,&mean1);
			  get_valid_stats(&Data1[(1-datahalf)*SAMPLE_LENGTH],Data_quality, &std1,&mean1,&valid_samples);

			  mycandata1.u32data.data0 =mean1;
			  mycandata1.u16data.data_u2 =std1;
			  mycandata1.u16data.data_u3 =valid_samples;
			  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			  HAL_CAN_AddTxMessage(&hcan,&txHeader1,mycandata1.stmp,&canMailbox);

		   	  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); //blue LED off

	//		  get_stats(&Data2[(1-datahalf)*SAMPLE_LENGTH],&std2,&mean2);
	//		  get_valid_stats(&Data2[(1-datahalf)*SAMPLE_LENGTH],Data_quality, &std2,&mean2,&valid_samples);
	//		  mycandata2.u32data.data0 =mean2;
	//		  mycandata2.u16data.data_u2 =std2;
	//		  mycandata2.u16data.data_u3 =valid_samples;
	//		  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
	//		  HAL_CAN_AddTxMessage(&hcan,&txHeader2,mycandata2.stmp,&canMailbox);

	//	   	  get_stats(&Data3[(1-datahalf)*SAMPLE_LENGTH],&std3,&mean3);
	//		  get_valid_stats(&Data3[(1-datahalf)*SAMPLE_LENGTH],Data_quality, &std3,&mean3,&valid_samples);
	//		  mycandata3.u32data.data0 =mean3;
	//		  mycandata3.u16data.data_u2 =std3;
	//		  mycandata3.u16data.data_u3 =valid_samples;
	//		  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
	//		  HAL_CAN_AddTxMessage(&hcan,&txHeader3,mycandata3.stmp,&canMailbox);

			  // report the number of amplitude errors.
			//  mycandata7.u16data.data_u0 =AEL[0];
			//  mycandata7.u16data.data_u1 =AEL[1];
			//  mycandata7.u16data.data_u2 =AEL[2];
			//  mycandata7.u16data.data_u3 =AEL[3];
			//  HAL_CAN_AddTxMessage(&hcan,&txHeader7,mycandata7.stmp,&canMailbox);

			  mycandata8.u16data.data_u0 =AEH[0];
			  mycandata8.u16data.data_u1 =AEH[1];
	//		  mycandata8.u16data.data_u2 =AEH[2];
	//		  mycandata8.u16data.data_u3 =AEH[3];
			  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {} // wait for a free mailbox.
			  HAL_CAN_AddTxMessage(&hcan,&txHeader8,mycandata8.stmp,&canMailbox);
			  AEH[0]=0;
			  AEH[1]=0;
	//		  AEH[2]=0;
	//		  AEH[3]=0;
	//		  for(i=0;i<CHANNEL_NUM;i++){
	//		  if(AEL[i]>5)
	//			AEL[i]-= 5;
	//		  else
	//			AEL[i]=0;

	//		  if(AEL[i]>100){
	//			  drivecurrent[i]++;
	//			  if(drivecurrent[i] >Max_Drive_Current)
	//				  drivecurrent[i] =Max_Drive_Current;
	//			  LDC1614_set_sensor_config(0xC601); //enter sleep
	//			  LDC1614_set_driver_current(i, drive_current_to_hex(drivecurrent[i]) );
	//			  LDC1614_set_sensor_config(0xD601); //resume
	//			  AEL[i]=0;
	//		  }

	//		  if(AEH[i]>5)
	//			AEH[i]-= 5;
	//		  else
	//			AEH[i]=0;

	//		  if(AEH[i]>100){
	//			  drivecurrent[i]--;
	//			  if(drivecurrent[i] <Min_Drive_Current)
	//				  drivecurrent[i] =Min_Drive_Current;
	//			  LDC1614_set_sensor_config(0xC601);
	//			  LDC1614_set_driver_current(i, drive_current_to_hex(drivecurrent[i]) );
	//			  LDC1614_set_sensor_config(0xD601);
	//			  AEH[i]=0;
	//		  }

	//		  }

			//	sprintf(Buffer, "Data0: %ld , Data1: %ld , Data2: %ld , Data3: %ld  \r\n",sen0_ch0,sen0_ch1,sen0_ch2,sen0_ch3);
			//	HAL_UART_Transmit(&huart2, Buffer, sizeof(Buffer), 10000);

			//	sprintf(Buffer, "status0: %d , status1: %d , status2: %d , status3: %d , Status: %ld \r\n",status0,status1,status2,status3,LDC1614_get_sensor_status());
			//	HAL_UART_Transmit(&huart2, Buffer, sizeof(Buffer), 10000);

			  //Last_Time=__HAL_TIM_GET_COUNTER(&htim2);
		  }
		 // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //blue LED
		 // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);




  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 2;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x0000020B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 16000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 160-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1)
{
	HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, MyRxcan.stmp); //Receive CAN bus message to canRX buffer
	if(Base_Can_address+0xA==rxHeader.StdId){


		 if(MyRxcan.stmp[0]==0){ // do nothing.
		 //drivecurrent[0]=Base_Drive_Current;
		 //drivecurrent[1]=Base_Drive_Current;
		 //drivecurrent[2]=Base_Drive_Current;
		 //drivecurrent[3]=Base_Drive_Current;
			// AEL[0]=0;
			 AEH[0]=0;
			// AEL[1]=0;
			 AEH[1]=0;
			// AEL[2]=0;
//			 AEH[2]=0;
//			// AEL[3]=0;
//			 AEH[3]=0;
		 }
		 else{
			 LDC1614_set_sensor_config(0xC601); //enter sleep
			 //
//			 drivecurrent[0]=MyRxcan.u16data.data_u0; //canRX[2*j]+(canRX[2*j+1]<<8);
//			 drivecurrent[1]=MyRxcan.u16data.data_u1;
//			 drivecurrent[2]=MyRxcan.u16data.data_u2;
//			 drivecurrent[3]=MyRxcan.u16data.data_u3;
			 for(int j=0;j<CHANNEL_NUM;j++){
				 drivecurrent[j]=MyRxcan.stmp[j];
			 if(drivecurrent[j] > Max_Drive_Current){
				 drivecurrent[j]=Max_Drive_Current;
			 }
			 if(drivecurrent[j] < Min_Drive_Current){
				 drivecurrent[j] =Min_Drive_Current;
			 }
			 //AEL[j]=0;
			 AEH[j]=0;
		 }
		LDC1614_reset_sensor();
		LDC1614_mutiple_channel_config();
		 }

	}
//	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);// toggle PA3 LED
//	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); //blue LED

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
