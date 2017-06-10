#ifndef IMU_H__
#define IMU_H__

#define IMU_ADDR 0x6A 
#define LSM_ARRAY_LEN 14 

void LSM6DS33_init(void);

unsigned char WhoAmI(void);

void I2C_read_multiple(unsigned char, unsigned char, unsigned char *, unsigned char); 

signed short Temp(unsigned char *); 
float convertTemp(unsigned char *); 

signed short xXL(unsigned char *);
signed short yXL(unsigned char *); 
signed short zXL(unsigned char *);
signed short xGyro(unsigned char *); 
signed short yGyro(unsigned char *); 
signed short zGyro(unsigned char *);

float convertxXL(unsigned char *); 
float convertyXL(unsigned char *);
float convertzXL(unsigned char *); 
float convertxGyro(unsigned char *); 
float convertyGyro(unsigned char *);
float convertzGyro(unsigned char *);

#endif