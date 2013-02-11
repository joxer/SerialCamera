#include "ff.h"
#include "pic24f.h"
#include "uart.h"
#include "monitor.h"
#include "diskio.h"
#include "ffhelper.h"
#include "serialcamera.h"

void reset_camera(){
	
		char dbuff[12];
		int i;	
		
		// camera reset code
		const unsigned char sync_word[4] = {0x56,0x00,0x26,0x00};
		
		send_command(sync_word,4);
		
		DelayMs(10);
		recv_message(UartDebug,4);
		
		
		
#ifdef DEBUG
		UARTWrite(1,"### Reset Camera ###\r\n");
		for(i = 0; i <4;i++){
			sprintf(dbuff,"curr: %x\r\n",(unsigned char)(UartDebug[i]));
			UARTWrite(1,dbuff);
			vTaskDelay(5);
		}
#endif
	DelayMs(100);
}

void take_picture(){
	
	char dbuff[12];
	int i;	
	// camera take picture command
	const unsigned char take_picture_command[5] = {0x56,0x00,0x36,0x01,0x00};
		
	send_command(take_picture_command,5);
	DelayMs(20);
	recv_message(UartDebug,5);
		
#ifdef DEBUG	
	UARTWrite(1,"### take_picture ###\r\n");
	for(i = 0; i <5;i++){
		sprintf(dbuff,"curr: %x\r\n",(unsigned int)UartDebug[i]);
		UARTWrite(1,dbuff);
		vTaskDelay(5);
	}
#endif
	DelayMs(100);
}

// TODO: dunno why it doesn't work

void set_compression(char compression){
		char dbuff[12];
		int i;	
		const unsigned char take_picture_command[9] = {0x56, 0x00, 0x31, 0x05, 0x01, 0x01, 0x12, 0x04, 0x35 };
		
		send_command(take_picture_command,9);
		DelayMs(20);
		recv_message(UartDebug,6);
		
#ifdef DEBUG
		UARTWrite(1,"### set_compression ###\r\n");
		for(i = 0; i <6;i++){
			sprintf(dbuff,"curr: %x\r\n",(unsigned int)UartDebug[i]);
			UARTWrite(1,dbuff);
			vTaskDelay(5);
		}
#endif		
}

void set_image_size_320x240(){
	
	
	char dbuff[12];
	int i;	
		
	// camera size command
	const unsigned char take_picture_command[9] = {0x56,0x00,0x31,0x05,0x04,0x01,0x00,0x19,0x11};
		
	send_command(take_picture_command,9);
	DelayMs(20);
	recv_message(UartDebug,5);
		
#ifdef DEBUG
	UARTWrite(1,"### set_image_size_320x240 ###\r\n");
	for(i = 0; i <5;i++){
		sprintf(dbuff,"curr: %x\r\n",(unsigned int)UartDebug[i]);
		UARTWrite(1,dbuff);
		vTaskDelay(5);
	}
#endif
	DelayMs(100);
}

void set_image_size_640x480(){
	
	char dbuff[12];
	int i;	
	// camera size command
	const unsigned char take_picture_command[9] = {0x56,0x00,0x31,0x05,0x04,0x01,0x00,0x19,0x00};
	
	send_command(take_picture_command,5);
	DelayMs(20);
	recv_message(UartDebug,5);
		
#ifdef DEBUG
	UARTWrite(1,"### set_image_size_640x480 ###\r\n");
	for(i = 0; i <5;i++){
		sprintf(dbuff,"curr: %x\r\n",(unsigned int)UartDebug[i]);
		UARTWrite(1,dbuff);
		vTaskDelay(5);
	}
#endif
	DelayMs(100);
}

void close_picture(){
	
	char dbuff[12];
	int i;	
	
	// camera end of transmission
	const unsigned char take_picture_command[5] = {0x56,0x00, 0x36,0x01,0x03};
		
	send_command(take_picture_command,5);
	DelayMs(10);
	recv_message(UartDebug,5);
		
#ifdef DEBUG
	UARTWrite(1,"### close_picture ###\r\n");
	for(i = 0; i <5;i++){
		sprintf(dbuff,"curr: %x\r\n",(unsigned int)UartDebug[i]);
		UARTWrite(1,dbuff);
		vTaskDelay(5);
	}
#endif
		DelayMs(50);
	
}


int get_length(){
	
	char dbuff[12];
	unsigned char size_buffer[9];
	const unsigned char size_command[5] = {0x56,0x00,0x34,0x01,0x00};
	int i;
	UARTFlush(2);
	send_command(size_command,5);
	DelayMs(10);
	recv_message(size_buffer,9);

	// address is in position 7 and 8 of the buffer
	
	int total = 0;
	total |= size_buffer[7]&0xff;
	total = total << 8;
	total |= size_buffer[8]&0xff;
#ifdef DEBUG
	UARTWrite(1,"### get_length ###\r\n");
	sprintf(dbuff,"total: %d",total);
	UARTWrite(1,dbuff);
#endif
	DelayMs(50);
	return total;
}

void transfer_picture(int size){
	
	
#ifdef DEBUG
	UARTWrite(1,"### transfer_picture ###\r\n");
#endif
	
    int i, j;	
	unsigned int writ = 0;
	unsigned int count = size/PIC_BUF_LEN;
	int tail = size%PIC_BUF_LEN;
	char dbuff[16];
	unsigned int addr = 0;
	unsigned  char buffer_write[PIC_BUF_LEN],tmp_buffer_write[10+PIC_BUF_LEN];
	
	/* camera command is defined as
	0x56 0x00 0x32 0x0C 0x00 0x0A 0x00 0x00
	0xAH 0xAL 0x00 0x00 0xLH 0xLL 0xXX 0xXX 
	
	0xAH 0xAL are the HIGH and LOW offset of memory
	0xLH 0xLL are the size of memory to get
	0xXX 0xXX time to wait between command
	
	0x0A is the default wait time
	
	*/
	unsigned char transfer_picture_command[16] ={0x56, 0x00, 0x32, 0x0C, 0x00,		
													0x0A,0x0A, 0x00, 0x00, 0x00,
													0x00, 0x00, 0x00, 0x00, 0x00,
													0x0A };
													
	// setting to start from first location of memory
													
	transfer_picture_command[8] = 0;
    transfer_picture_command[9] = 0;
	
	// set PIC_BUF_LEN as length of memory to get
	
	transfer_picture_command[13] = PIC_BUF_LEN;		
	
    for(i = 0; i < count; i++){
	  
  	  send_command(transfer_picture_command,16);
      DelayMs(8);
  	  recv_message(tmp_buffer_write,10+PIC_BUF_LEN);
      
	  for(j = 0; j < PIC_BUF_LEN;j++){
		  buffer_write[j] = (tmp_buffer_write+5)[j] & 0xff;		  
	  }
	  
	  f_write(&FILE_CAM,buffer_write,PIC_BUF_LEN,&writ);
#ifdef DEBUG
	  sprintf(dbuff,"len: %d\n", writ);
	  UARTWrite(1,dbuff);
#endif
	  //  sprintf(dbuff,"%x \n",addr);
	 // UARTWrite(1,dbuff);
	  addr += PIC_BUF_LEN;
      transfer_picture_command[8] = addr >> 8;
      transfer_picture_command[9] = addr & 0x00FF;
	  f_lseek(&FILE_CAM,addr);
	}
	transfer_picture_command[13] = tail;
	send_command(transfer_picture_command,16);
	DelayMs(10);
	recv_message(tmp_buffer_write,10+tail);
	unsigned char new_buffer_write[tail];
    
	for(j = 0; j < tail;j++){
	  new_buffer_write[j] = (tmp_buffer_write+5)[j] & 0xff;
	}
	
	f_write(&FILE_CAM,new_buffer_write,PIC_BUF_LEN,&writ);
	f_close(&FILE_CAM);
#ifdef DEBUG
	sprintf(dbuff,"len: %d", writ);
	UARTWrite(1,dbuff);
#endif
	DelayMs(50);
	//vTaskDelay(20);
	
}

void sync_cam()
{
#ifdef DEBUG			
	UARTWrite(1,"### sync_cam() ###\r\n");
#endif	
	// sync cam with flyport
	
	UARTWrite(1,"Synchronizing\r\n");
	UARTInit(2,115200);  	
	UARTOn(2);
}


void save_photo_to_sd(){		
	
#ifdef DEBUG			
	UARTWrite(1,"### save_photo_on_sd() ###\r\n");
#endif
	
	//if (UARTucam_inited == first) UARTucam_inited = notfirst;
//	else { UARTOff(UART);}
	sync_cam();
	acc_files = 0;
	int number = get_number_of_file_on_sd();
	acc_files = 0;
	char dbuff[12];
	sprintf(dbuff,"image%d.jpeg",number);
    f_open(&FILE_CAM, dbuff, FA_CREATE_ALWAYS|FA_WRITE);
    reset_camera();
	take_picture();
	
	int len = get_length();
	if(len > 0){
		transfer_picture(len);
		close_picture();
	}
	UARTWrite(1,"finished\n");
}

int recv_message(unsigned char* buffer,int length)
{

	unsigned char buff[1];
	int i;
	while(1){
	   if(UARTBufferSize(2) >= length){
			for(i = 0; i < length;i++){
				UARTRead(2,(unsigned char*)buff,1);
				IOPut( 19,toggle);

				buffer[i] = ((unsigned char)buff[0]&0xff);
			}
#ifdef DEBUG			
	UARTWrite(1,"### received message ###\r\n");
	sprintf(UartDebug, "len: %d\n", length);
	UARTWrite(1,UartDebug);
#endif
		return 0;
		}
	}

	return 0;
}

void send_command( const unsigned char* com,int length)
{
	#ifdef DEBUG			
		UARTWrite(1,"### send_command ###\r\n");
	#endif
	
	UARTFlush(2);

	
	int i;
	for (i=0;i<length;i++)
	{
		UARTWriteCh(2,*(char*)(com + i));
	}
}

int get_number_of_file_on_sd(){
#ifdef DEBUG
	UARTWrite(1,"### get_number_of_file_on_sd ###\r\n");
#endif
	file_rep = listFileSD("/", 0);
	return acc_files;
}

int get_last_file(){
	
	unsigned int i = 0;
	char dbuff[16];
	
	// 65536 is the number of possible file on a fat filesystem inside a directory
	
	while(i < 65535){
		sprintf(dbuff,"image%d.jpeg",i);
		FIL info;
		if (f_open(&info,dbuff,FA_READ) == FR_NO_FILE){
			return --i;
		}
		i++;
	}
	return -1;
}
