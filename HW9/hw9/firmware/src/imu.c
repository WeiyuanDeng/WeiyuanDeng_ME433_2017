#include <xc.h>
#include"i2c_master_noint.h" 
#include"imu.h" 

void LSM6DS33_init(void) { 
    i2c_master_setup(); 
    i2c_master_start(); 
    i2c_master_send(IMU_ADDR << 1);  
    i2c_master_send(0x10); 
    i2c_master_send(0x82); 
    i2c_master_start();
    i2c_master_send(IMU_ADDR << 1); 
    i2c_master_send(0x11); 
    i2c_master_send(0x88);  
    i2c_master_stop(); 
    i2c_master_start();
    i2c_master_send(IMU_ADDR << 1); 
    i2c_master_send(0x12);
    i2c_master_send(0x04);  
    i2c_master_stop(); 
}

unsigned char WhoAmI(void) { 
    unsigned char answer;
    i2c_master_start();
    i2c_master_send((IMU_ADDR << 1));
    i2c_master_send(0x0F);  
    i2c_master_restart();
    i2c_master_send((IMU_ADDR << 1) | 1); 
    answer = i2c_master_recv();
    i2c_master_ack(1); 
    i2c_master_stop();
    return answer;
}

void I2C_read_multiple(unsigned char address, unsigned char registerrr, unsigned char * data, unsigned char length) {
    char i;
        i2c_master_start();
        i2c_master_send((address << 1)); 
        i2c_master_send(registerrr);  
        i2c_master_restart(); 
        i2c_master_send((address << 1) | 1); 
        for (i = 0; i < length; i++) {
            data[i] = i2c_master_recv(); 
            if (i==(length-1)) {
                i2c_master_ack(1); 
            }
            else {
                i2c_master_ack(0); 
            }
        }
        i2c_master_stop();
}

signed short Temp(unsigned char * data) { 
    return data[1]<<8 | data[0];
}

signed short xGyro(unsigned char * data) {
    return data[3]<<8 | data[2];
}

signed short yGyro(unsigned char * data) {
    return data[5]<<8 | data[4];
}

signed short zGyro(unsigned char * data) { 
    return data[7]<<8 | data[6];
}

signed short xXL(unsigned char * data) { 
    return data[9]<<8 | data[8];
}

signed short yXL(unsigned char * data) { 
    return data[11]<<8 | data[10];
}

signed short zXL(unsigned char * data) { 
    return data[13]<<8 | data[12];
}

float convertTemp(unsigned char * data) { 
    return (Temp(data)+400)/16.0;
}

float convertxXL(unsigned char * data) { 
    return (xXL(data))*0.000061;
}

float convertyXL(unsigned char * data) { 
    return (yXL(data))*0.000061;
}

float convertzXL(unsigned char * data) { 
    return (zXL(data))*0.000061;
}

float convertxGyro(unsigned char * data) { 
    return (xGyro(data))*0.035;
}

float convertyGyro(unsigned char * data) { 
    return (yGyro(data))*0.035;
}

float convertzGyro(unsigned char * data) {
    return (zGyro(data))*0.035;
}
