/*
    Seeed_LDC1612.cpp
    Driver for DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR

    Copyright (c) 2018 Seeed Technology Co., Ltd.
    Website    : www.seeed.cc
    Author     : downey
    Create Time: June 2018
    Change Log :

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/


#include "LDC_1614.h"
#include "math.h"
#include "fpsqrt.h"
#include <stdlib.h>
/** @brief read sensor's information


void LDC1614_read_sensor_infomation() {
    u16 value = 0;
    IIC_read_16bit(READ_MANUFACTURER_ID, &value);
    Serial.print("manufacturer id =0x");
    Serial.println(value, HEX);
    IIC_read_16bit(READ_DEVICE_ID, &value);
    Serial.print("DEVICE id =0x");
    Serial.println(value, HEX);
    return ;
}
 * */

/** @brief constructor,support set IIC addr,default iic addr is 0x2b.


LDC1614_LDC1612(u8 IIC_ADDR) {
    set_iic_addr(IIC_ADDR);
}
 * */

/** @brief config sensor

 * */
//float LDC1614_resistance[CHANNEL_NUM];
//float LDC1614_inductance[CHANNEL_NUM];
//float LDC1614_capacitance[CHANNEL_NUM];
//float LDC1614_Fref[CHANNEL_NUM];
//float LDC1614_Fsensor[CHANNEL_NUM];
//float LDC1614_Q_factor[CHANNEL_NUM];
extern uint8_t drivecurrent[2];


s32 LDC1614_mutiple_channel_config() {
    /*Set coil inductor parameter first.*/

//	for(int i=0; i<CHANNEL_NUM;i++)
//	{	/*28 TURNS*/

 //  LDC1614_resistance[i] = 28.88;
//   LDC1614_inductance[i] = 78.3;

 //  LDC1614_capacitance[i] = 270;

  // LDC1614_Q_factor[i] = 52.5;


//	}


    if (LDC1614_set_FIN_FREF_DIV(0)) {
        return -1;
    }
    LDC1614_set_FIN_FREF_DIV(1);
//    LDC1614_set_FIN_FREF_DIV(2);
//    LDC1614_set_FIN_FREF_DIV(3);

    /* 16*38/Fref=30us ,wait 30us for LC sensor stabilize before initiation of a conversion.*/


	for(uint8_t i=0; i<CHANNEL_NUM;i++)
	{

	LDC1614_set_LC_stabilize_time(i);
    /*Set conversion interval time 20 SPS should give 0.05um resolution*/
	//LDC1614_set_conversion_time(i, 0xF474);//9.96Hz  62580
	//LDC1614_set_conversion_time(i, 0xF000);//10.14 Hz 61440
	LDC1614_set_conversion_time(i, 0xF220);//10 Hz
	//LDC1614_set_conversion_time(i, 0x78Ef);//20 Hz
// 0x0005-0xFFFF: Conversion Time (tC1)= (RCOUNT1×16)/ƒREF1

    /*Set driver current!*/
	LDC1614_set_driver_current(i, drive_current_to_hex(drivecurrent[i]) ); //0x7000 14 b01110 127uA 7.42-11.2 Kohm sensor RP

	}
	//LDC1614_set_driver_current(3, 0x6800); //increase the drive current for close connection on the green boards.

    /*mutiple conversion 1000001000001100*/
	// LDC1614_set_mux_config(0x820c);
    /*mutiple conversion 1100001000001100*/
	//LDC1614_set_mux_config(0xC20C); // ALL 4 CHANNELS
	//LDC1614_set_mux_config(0x020C); // ONLY CHANNEL 03
	LDC1614_set_mux_config(0x8209); // ONLY CHANNEL 00,01
    //set_mux_config(0x20c);
	//	LDC1614_set_ERROR_CONFIG
	//LDC1614_set_ERROR_CONFIG(0xF819); // set intB data ready and AEL and AEH
	LDC1614_set_ERROR_CONFIG(0xF801); // set intB data ready only
	//LDC1614_set_ERROR_CONFIG(0xE811); // set intB data ready only and AEH 1110 1000 0001 0001
    /*start channel 0*/
	LDC1614_set_sensor_config(0xD601); // external 40mhz clock
	//LDC1614_set_sensor_config(0xD401); // internal clock

    //u16 config=0x1601;
    //select_channel_to_convert(0,&config);



    return 0;
}


/** @brief parse the data which read from data register.
    @param channel LDC1612 has total two channels.
    @param raw_result the raw data which read from data register,it contains error codes and sensor value;
 * */
int32_t LDC1614_parse_result_data(uint8_t channel, uint32_t raw_result, uint32_t* result) {
	uint8_t value = 0;
    *result = raw_result & 0x0fffffff;
    if (0xfffffff == *result) {
        //Serial.println("can't detect coil Coil Inductance!!!");
        return *result;
    }
    // else if(0==*result)
    // {
    //     Serial.println("result is none!!!");
    // }
    value = raw_result >> 28;
    return value;
/*
    if (value & 0x8) {
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_UR-Under range error!!!");
        return value;
    }
    if (value & 0x4) {
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_OR-Over range error!!!");
        return value;
    }
    if (value & 0x2) {
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_WD-Watch dog timeout error!!!");
        return value;
    }
    if (value & 0x1) {
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_AE-error!!!");
        return value;
    }
    return 0;*/
}


/** @brief read the raw channel result from register.
    @param channel LDC1612 has total two channels.
    @param result raw data
 * */
s32 LDC1614_get_channel_result(u8 channel, uint32_t* result) {
    uint32_t raw_value = 0;
    if (NULL == result) {
        return -1;
    }
    u16 value = 0;
    IIC_read_16bit(CONVERTION_RESULT_REG_START + channel * 2, &value);

    raw_value |= (uint32_t)value << 16;
    IIC_read_16bit(CONVERTION_RESULT_REG_START + channel * 2 + 1, &value);
    raw_value |= (uint32_t)value;
    return LDC1614_parse_result_data(channel, raw_value, result);
    //raw_value >> 28;
}

/** @brief set conversion interval time.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
s32 LDC1614_set_conversion_time(u8 channel, u16 value) {
    return IIC_write_16bit(SET_CONVERSION_TIME_REG_START + channel, value);
}


/** @brief set conversion offset.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
s32 LDC1614_set_conversion_offset(u8 channel, u16 value) {
    return IIC_write_16bit(SET_CONVERSION_OFFSET_REG_START + channel, value);
}

/** @brief Before conversion,wait LC sensor stabilize for a short time.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
s32 LDC1614_set_LC_stabilize_time(u8 channel) {
    u16 value = 0;
    value = 0x1FF;//155//30; 3200 for settle count or 0x00C8
    // value = 155;
    return IIC_write_16bit(SET_LC_STABILIZE_REG_START + channel, value);
}


/** @brief set input frequency divide and fref divide.
    @param channel LDC1612 has total two channels.
    @param FIN_DIV FIN input divide
    @param FREF_DIV fref,reference frequency of sensor.
 * */
s32 LDC1614_set_FIN_FREF_DIV(u8 channel) {
    u16 value;
    u16 FIN_DIV, FREF_DIV;

  //  LDC1614_Fsensor[channel] = 1 / (2 * 3.14 * sqrt(LDC1614_inductance[channel] * LDC1614_capacitance[channel] * pow(10, -18))) * pow(10, -6); 1.0952 MHZ
//    Serial.print("fsensor =");
//    Serial.println(Fsensor[channel]);


    FIN_DIV = 1;//(u16)(LDC1614_Fsensor[channel] / 8.75 + 1);
    FREF_DIV = 1;

   // if (LDC1614_Fsensor[channel] * 4 < 40) {
    //    FREF_DIV = 2;
    //    LDC1614_Fref[channel] = 40 / 2;
   // } else {
    //    FREF_DIV = 4;
   //     LDC1614_Fref[channel] = 40 / 4;
   // }

    value = FIN_DIV << 12;
    value |= FREF_DIV;
    return IIC_write_16bit(SET_FREQ_REG_START + channel, value);
}


/** @brief Error output config.
    @param result The value to be set.
 * */
s32 LDC1614_set_ERROR_CONFIG(u16 value) {
    return IIC_write_16bit(ERROR_CONFIG_REG, value);
}



/** @brief mux  config.
    @param result The value to be set.
 * */
s32 LDC1614_set_mux_config(u16 value) {
    return IIC_write_16bit(MUL_CONFIG_REG, value);
}

/** @brief reset sensor.

 * */
s32 LDC1614_reset_sensor() {
    return IIC_write_16bit(SENSOR_RESET_REG, 0x8000);
}

/** @brief set drive current of sensor.
    @param result The value to be set.
 * */
s32 LDC1614_set_driver_current(u8 channel, u16 value) {
    return IIC_write_16bit(SET_DRIVER_CURRENT_REG + channel, value);
}


/** @brief Main config part of sensor.Contains select channel、start conversion、sleep mode、sensor activation mode、INT pin disable ..
    @param result The value to be set.
 * */
s32 LDC1614_set_sensor_config(u16 value) {
    return IIC_write_16bit(SENSOR_CONFIG_REG, value);
}


/** @brief select channel to convert

 * */
void LDC1614_select_channel_to_convert(u8 channel, u16* value) {
    switch (channel) {
        case 0: *value &= 0x3fff;
            break;
        case 1: *value &= 0x7fff;
            *value |= 0x4000;
            break;
        case 2: *value &= 0xbfff;
            *value |= 0x8000;
            break;
        case 3: *value |= 0xc000;
            break;
    }
}





const char* status_str[] = {"conversion under range error", "conversion over range error",
                            "watch dog timeout error", "Amplitude High Error",
                            "Amplitude Low Error", "Zero Count Error",
                            "Data Ready", "unread conversion is present for channel 0",
                            " unread conversion is present for Channel 1.",
                            "unread conversion is present for Channel 2.",
                            "unread conversion is present for Channel 3."
                           };




uint32_t LDC1614_get_sensor_status() {
    u16 value = 0;
    IIC_read_16bit(SENSOR_STATUS_REG, &value);

    // Serial.print("status =");
    // Serial.println(value,HEX);

//    LDC1614_sensor_status_parse(value); // FixME
    return value;
}


void get_stats(uint32_t *data,uint32_t *std_dev,uint32_t *mean){
  *std_dev=0;
  *mean=0;
  unsigned long long mean64=0;
  int i;
  int nvalid=0;
 // Drop zeros from and invalid data.
 // using index_t = decltype(*data)::index_t;
    for (i = 0; i < SAMPLE_LENGTH; i++)
    {
    	if(data[i]>0 && data[i]<0x0FFFFFFF){
			mean64 += data[i];
			nvalid++;
    	}

    }
    *mean=mean64/ (nvalid);

    for(i=0; i < SAMPLE_LENGTH; i++) {
    	if(data[i]>0 && data[i]<0x0FFFFFFF){
    		*std_dev += (data[i] - *mean)*(data[i] - *mean);
    	}
    }

    *std_dev= sqrt_i32(*std_dev/(nvalid));
}

void get_valid_stats(uint32_t *data,uint32_t *valid, uint32_t *std_dev,uint32_t *mean,int* nvalid){
  u32 old_mean=*mean;
  unsigned long long mean64=0;
  *mean=0;
  int i_index;
  *nvalid=0;

  *std_dev=MAX(2,*std_dev);
   for(i_index=0;i_index<SAMPLE_LENGTH;i_index++){



          if (abs(data[i_index] - old_mean) < threshold * (*std_dev)&&data[i_index]>0 && data[i_index]<0x0FFFFFFF) {
                //Data_0_quality[idx]=1;// data is good, less than 1 std dev.
                SetBit(valid,i_index);

                //Serial.print(valid[i]);
                mean64 += data[i_index] ;
                (*nvalid)++;
              }
          else{
               // Data_0_quality[idx]=0;
                ClearBit(valid,i_index);
          }

             //   Serial.print(TestBit(valid,i_index));
             //   Serial.print(",");

     }
      //  Serial.println();
      //  Serial.print("nvalid");
      //  Serial.println(*nvalid);

    *mean=mean64/( *nvalid);
   // Serial.print("newmean:");
   // Serial.println(*mean);

    *std_dev=0;
    for(i_index=0; i_index < SAMPLE_LENGTH; i_index++) {
      if(TestBit(valid,i_index) ){
        *std_dev += ( data[i_index] - *mean)*( data[i_index] - *mean);
      }
    }

    *std_dev= sqrt_i32( *std_dev/((*nvalid)) );
}

void IIC_read_16bit(u8 start_reg, u16* value) {
    u8 receive_data[2] = {0,0};
    *value = 0;
   // Wire1.beginTransmission(_IIC_ADDR);
   // Wire1.write(start_reg);
   // Wire1.endTransmission(false);

//    Wire1.requestFrom(_IIC_ADDR, sizeof(u16));
 //   while (sizeof(u16) != Wire1.available());
  //  val = Wire1.read();
  //  *value |= (u16)val << 8;
  //  val = Wire1.read();
  //  *value |= val;


    //trigger reading
	HAL_I2C_Master_Transmit(&hi2c1, LDC_IIC_ADDR<<1, &start_reg, 1, 1000);

	//conversion takes 6.50ms(temperature)
	//HAL_Delay(10);

	//Read temperature and humidity
	if(HAL_I2C_Master_Receive(&hi2c1, LDC_IIC_ADDR<<1, receive_data, 2, 2000)== HAL_ERROR)
	{
		HAL_Delay(1);
		HAL_I2C_Master_Receive(&hi2c1, LDC_IIC_ADDR<<1, receive_data, 2, 2000);
	}

	*value =  ( (receive_data[0]<<8) | receive_data[1] );


}

s32 IIC_write_16bit(u8 reg, u16 value) {
//    Wire1.beginTransmission(_IIC_ADDR);
	//u8
	//uint8_t ret;
//    ret = HAL_I2C_IsDeviceReady(&hi2c1, LDC_IIC_ADDR<<1, 2, 5);
//    if (ret != HAL_OK){ /* No ACK Received At That Address */
//    	return ret;
//    }

	//uint16_t	configuration = 0;
	uint8_t		config_arr[2];

	//configuration = ( RST | MODE | TRES(TRES_14BIT) | HRES(HRES_14BIT) );

	config_arr[0] = (value & 0xFF00)>>8;	//configuration MSB
	config_arr[1] = (value & 0x00FF);	//configuration LSB

	return HAL_I2C_Mem_Write(&hi2c1, LDC_IIC_ADDR<<1, reg, I2C_MEMADD_SIZE_8BIT, config_arr, 2, 100);


    // HAL_I2C_Master_Transmit(&hi2c1, LDC_IIC_ADDR<<1, &reg, 1, 1000);
    // return HAL_I2C_Master_Transmit(&hi2c1, LDC_IIC_ADDR<<1, (u8*) &value, 2, 2000);


}


/**********************************************************************************************************/
/************************************************IIC PART************************************************/
/*********************************************************************************************************

s32 LDC1612_IIC_OPRTS::IIC_write_byte(u8 reg, u8 byte) {
    Wire1.beginTransmission(_IIC_ADDR);
    Wire1.write(reg);
    Wire1.write(byte);
    return Wire1.endTransmission();
}


s32 LDC1612_IIC_OPRTS::IIC_write_16bit(u8 reg, u16 value) {
    Wire1.beginTransmission(_IIC_ADDR);
    Wire1.write(reg);

    Wire1.write((u8)(value >> 8));
    Wire1.write((u8)value);
    return Wire1.endTransmission();
}


void LDC1612_IIC_OPRTS::IIC_read_byte(u8 reg, u8* byte) {
    Wire1.beginTransmission(_IIC_ADDR);
    Wire1.write(reg);
    Wire1.endTransmission(false);

    Wire1.requestFrom(_IIC_ADDR, (u8)1);
    while (1 != Wire1.available());
    *byte = Wire1.read();

}

void LDC1612_IIC_OPRTS::IIC_read_16bit(u8 start_reg, u16* value) {
    u8 val = 0;
    *value = 0;
    Wire1.beginTransmission(_IIC_ADDR);
    Wire1.write(start_reg);
    Wire1.endTransmission(false);

    Wire1.requestFrom(_IIC_ADDR, sizeof(u16));
    while (sizeof(u16) != Wire1.available());
    val = Wire1.read();
    *value |= (u16)val << 8;
    val = Wire1.read();
    *value |= val;
}



void LDC1612_IIC_OPRTS::set_iic_addr(u8 IIC_ADDR) {
    _IIC_ADDR = IIC_ADDR;
}

*/
