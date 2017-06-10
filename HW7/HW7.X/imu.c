#include<proc/p32mx250f128b.h>
#include"imu.h" 

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C2 channel 
// I2C pins need pull-up resistors, 2k-10k

void i2c_master_setup(void) {
    ANSELBbits.ANSB2 = 0;   // Turn off analog for P32 pin B2
    ANSELBbits.ANSB3 = 0;   // Turn off analog for P32 pin B3
            
    I2C2BRG = 233;          // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 
                            // Fsck = 100kHz, PGD = 104ns, Pblck = 48MHz
    I2C2CONbits.ON = 1;     // turn on the I2C2 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
    I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
    while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
    if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
        // ("I2C2 Master: failed to receive ACK\r\n");
    }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
    I2C2CONbits.PEN = 1;              // comm is complete and master relinquishes bus
    while(I2C2CONbits.PEN) { ; }      // wait for STOP to complete
}

void init_expander(){
    //Turning on accelerometer 
    set_expander(CTRL1_XL, 0x82); //Sample rate 1.66kHz, 2g sensitivity, 100Hz filter
                                  //0b10000010
    set_expander(CTRL2_G, 0x88);  //Sample rate 1.66kHz, 1000dps sensitivity
                                  //0b10001000
    set_expander(CTRL3_C, 0b100); //IF_INC bit = 1, this is bit2 on register    
}

void set_expander(unsigned char address, unsigned char val){
    //opcode = 0100 0000 for write, opcode = 0100 0001 for read
    i2c_master_start();
    i2c_master_send(SLAVE_ADDR << 1); //device opcode write mode
    i2c_master_send(address);         //specify register address 
    i2c_master_send(val);             //data to slave
    i2c_master_stop();
}

unsigned char get_expander(unsigned char address){
    i2c_master_start();
    i2c_master_send(SLAVE_ADDR << 1);  //writing
    i2c_master_send(address);          //specifying register address
    i2c_master_restart();   
    
    i2c_master_send((SLAVE_ADDR << 1) | 1); //reading from register
    char info = i2c_master_recv();    //storing byte read into pic 
    i2c_master_ack(1);                //let slave know read successful
    i2c_master_stop();
    
    return info;
}

void i2c_read_multiple(unsigned char address, unsigned char r, unsigned char* data, int length){
    //address - chip address (remember to pass in SLAVE_ADDR in main)
    //r - register address
    int i;
    
    i2c_master_start();
    i2c_master_send(address << 1); // write mode?
    i2c_master_send(r);
    i2c_master_restart();
    
    i2c_master_send((address << 1) | 1);   //read mode
    for(i=0;i<length;i++){
        data[i] = i2c_master_recv(); // store returned info into data array
        if(i<length-1){
            i2c_master_ack(0); // keep reading
        }
        else{
            i2c_master_ack(1); // done reading
        }
    }
    i2c_master_stop();
}