#define PIC_BUF_LEN 176

unsigned char UartDebug[20];
FIL FILE_CAM;
void reset_camera();
void take_picture();
void set_compression(char );
void set_image_size_320x240();
void set_image_size_640x480();
void close_picture();
int get_length();
void transfer_picture(int);
void sync_cam();
int recv_message(unsigned char*,int );
void send_command( const unsigned char*,int);
int get_number_of_file_on_sd();
void save_photo_to_sd();
