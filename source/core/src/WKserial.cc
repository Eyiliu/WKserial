#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <limits.h>
#include <sys/ioctl.h>


#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

#include "WKserial.hh"

WKserial::WKserial(std::string name):
  m_fd(0),m_devname(name), m_endline("\n"){
  
}

WKserial::~WKserial(){
  disconnect();
}


int WKserial::get_fd(){
  return m_fd;
}



int WKserial::connect()
{
  m_fd = open( m_devname.c_str(), O_RDWR | O_NOCTTY); //O_NDELAY
  if (m_fd==-1){
    fprintf(stderr,"[WKserial] Error %s:  Can't Open Serial Port! \n", m_devname.c_str());
    return -1;
  }
  struct termios  options;
  tcgetattr(m_fd, &options);
  options.c_cc[VTIME] = 1; /* seconds, TODO*/
  options.c_cc[VMIN] = 0;  
  tcflush(m_fd,TCIOFLUSH);
  tcsetattr(m_fd, TCSANOW, &options);

  m_tp_connect=std::chrono::steady_clock::now();
  return m_fd;
}


void WKserial::disconnect(){
  if(m_fd!=0){
    close(m_fd);
  }
  m_fd=0;
}


void WKserial::set_write_interval_msec(int msec){
  m_du_interval_write = std::chrono::milliseconds(msec);
  return;
}

void WKserial::set_read_timeout_msec(int msec){
  m_du_timeout_read = std::chrono::milliseconds(msec);
  return;
}


void WKserial::set_endline(std::string el){
  m_endline = el;
}

void WKserial::set_speed(int speed){
  const static int speed_arr[] = {
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800,
    B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000,
    B2500000, B3000000, B3500000, B4000000,};
  
  const static int name_arr[] = {
    0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
    4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
    500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000,
    2500000, 3000000, 3500000, 4000000,};
  
  int   status; 
  struct termios   options;
  tcgetattr(m_fd, &options); 
  for(unsigned int i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
    if(speed == name_arr[i]) {
      tcflush(m_fd, TCIOFLUSH);
      cfsetispeed(&options, speed_arr[i]);  
      cfsetospeed(&options, speed_arr[i]);   
      status = tcsetattr(m_fd, TCSANOW, &options);  
      if(status != 0) {        
	fprintf(stderr,"[WKserial] Error %s:  tcsetattr fd1\n", m_devname.c_str());  
        return;     
      }
      tcflush(m_fd,TCIOFLUSH);   
    }  
  }
}

int WKserial::get_speed(){
  const static int speed_arr[] = {
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800,
    B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000,
    B2500000, B3000000, B3500000, B4000000,};
  
  const static int name_arr[] = {
    0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
    4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
    500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000,
    2500000, 3000000, 3500000, 4000000,};
  
  struct termios   options;
  tcgetattr(m_fd, &options);
  int speed = cfgetispeed(&options);
  int name = 0;
  for(unsigned int i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
    if(speed == speed_arr[i]) {
      name = name_arr[i];
    }
  }
  return name;
}


void WKserial::set_hardware_flow_control(int ison){
  // RS-232 signal lines CTS/RTS 
  struct termios   options;
  tcgetattr(m_fd, &options); 
  if(ison==0){                    //CRTSCTS also called  CNEW_RTSCTS;
    options.c_cflag &= ~CRTSCTS;   //disable hard
  }
  else{
    options.c_cflag |= CRTSCTS;    //enable hard
  }
  tcflush(m_fd,TCIOFLUSH);   
  tcsetattr(m_fd, TCSANOW, &options);
}

void WKserial::set_software_flow_control(int ison){
  struct termios   options;
  tcgetattr(m_fd, &options); 
  if(ison==0){
    options.c_iflag &= ~(IXON | IXOFF | IXANY);  //disable soft
  }
  else{
    options.c_iflag |= (IXON | IXOFF | IXANY);  //enable soft
  }
  tcflush(m_fd,TCIOFLUSH);   
  tcsetattr(m_fd, TCSANOW, &options);
}

void WKserial::set_raw_output(int israw){
  struct termios   options;
  tcgetattr(m_fd, &options); 
  if(israw==0){
    options.c_oflag |= OPOST;   // processed output
  }
  else{
    options.c_oflag &= ~OPOST;  // raw output, all other option bits in c_oflag are ignored
  }
  tcflush(m_fd,TCIOFLUSH);
  tcsetattr(m_fd, TCSANOW, &options);
}

void WKserial::set_raw_intput(int israw){
  struct termios   options;
  tcgetattr(m_fd, &options); 
  if(israw==0){
    options.c_lflag |= (ICANON | ECHO | ECHOE);
    //Canonical input is line-oriented.
    //Input characters are put into a buffer which can be edited interactively
    //by the user until a CR (carriage return) or LF (line feed) character is received. 
  }
  else{
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //raw input, input characters are passed through exactly as they are received
  }
  tcflush(m_fd,TCIOFLUSH);
  tcsetattr(m_fd, TCSANOW, &options);
}

void WKserial::set_parity(int databits,int stopbits,int parity){ 
  struct termios options;
  if(tcgetattr( m_fd,&options)  !=  0) { 
    fprintf(stderr,"[WKserial] Error %s:  SetupSerial 1\n", m_devname.c_str());
    return;  
  }
  //Enable the receiver and set local mode...
  options.c_cflag |= (CLOCAL | CREAD);
  //
  options.c_cflag &= ~CSIZE; 
  switch (databits)
    {   
    case 7:		
      options.c_cflag |= CS7; 
      break;
    case 8:     
      options.c_cflag |= CS8;
      break;   
    default:    
      fprintf(stderr,"[WKserial] Error %s:  Unsupported data size\n", m_devname.c_str());
      return;  
    }
  switch (parity) 
    {   
    case 'n':
    case 'N':
      options.c_cflag &= ~PARENB;   /* Clear parity enable */
      options.c_iflag &= ~INPCK;
      break;  
    case 'o':   
    case 'O':     
      options.c_cflag |= PARENB;
      options.c_cflag |= PARODD;
      options.c_iflag |= INPCK;
      options.c_iflag |= ISTRIP;
      break;  
    case 'e':  
    case 'E':
      options.c_cflag |= PARENB;
      options.c_cflag &= ~PARODD;
      options.c_iflag |= INPCK;
      options.c_iflag |= ISTRIP;
      break;
    default:   
      fprintf(stderr,"[WKserial] Error %s:  Unsupported parity\n", m_devname.c_str());    
      return;  
    }  
  switch (stopbits)
    {   
    case 1:    
      options.c_cflag &= ~CSTOPB;  
      break;  
    case 2:    
      options.c_cflag |= CSTOPB;  
      break;
    default:    
      fprintf(stderr,"[WKserial] Error %s:  Unsupported stop bits\n", m_devname.c_str());  
      return; 
    } 
  
  tcflush(m_fd,TCIFLUSH);
  if (tcsetattr(m_fd,TCSANOW,&options) != 0)   
    { 
      fprintf(stderr,"[WKserial] Error %s:  SetupSerial 3\n", m_devname.c_str());
      return;  
    } 
  return;  
}


void WKserial::write_data(std::string data)
{  
  auto tp_wait = m_tp_last_write + m_du_interval_write;
  if(tp_wait > std::chrono::steady_clock::now()){
    fprintf(stdout,"[WKserial] debug %s: write delay\n",m_devname.c_str());    
    std::this_thread::sleep_until(tp_wait);
  }
  printf("[WKserial] Write %s:  %lu bytes: %s\n", m_devname.c_str(), data.length(),data.c_str()); 
  write(m_fd, data.c_str(), data.length());
  m_tp_last_write = std::chrono::steady_clock::now();
}

void WKserial::write_line(std::string line)
{
  write_data(line+m_endline);
}

void WKserial::read_data(std::string &str, int length){
  str.clear();
  while(1){
    int length_waiting1, length_waiting2;
    ioctl(m_fd, FIONREAD, &length_waiting1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10 + 1000*100/get_speed()));
    //10ms + ~10bytes
    ioctl(m_fd, FIONREAD, &length_waiting2);
    if(length_waiting2 == 0){
      auto tp_wait = std::max(m_tp_connect, m_tp_last_write) + m_du_timeout_read;
      if(tp_wait > std::chrono::steady_clock::now()){
	continue; //waiting
      }
      else{ //timeout
	fprintf(stderr,"[WKserial] error %s: Can not read from device! Check the device power, connection or increase the read_timeout_msec!\n",
		m_devname.c_str());
	return;
      }
    }
    if(length_waiting1 == length_waiting2 && length_waiting2 != 0)
      break;  // success
  }
  
  char *buffer = new char[length];  
  int actual_len = read(m_fd, buffer, length);
  str.append(buffer, actual_len);
  printf("[WKserial] Read %s:  %d bytes: %s", m_devname.c_str(), actual_len, str.c_str());
  delete []buffer;
 
}


//// SCPI functions
void WKserial::scpi_send_command(std::string cmd){
  write_line(cmd);
}

void WKserial::scpi_send_data(std::string data){
  write_data(data);
}

void WKserial::scpi_receive_data(std::string &data, int maxlen){
  read_data(data, maxlen);
}

void WKserial::scpi_get_idn(std::string &idndata){
  const static std::string idncmd = "*IDN?";
  scpi_send_command(idncmd);
  scpi_receive_data(idndata, 200); //len=200 should be enough.  
}

void WKserial::scpi_print_idn(){
  std::string idndata;
  scpi_get_idn(idndata);
  std::cout<<"[SCPI] IDN data in "<<m_devname<<" is:  "
	   <<idndata.length()<<" bytes"<<std::endl
	   <<idndata<<std::endl;
}

int WKserial::scpi_is_device(std::string idnsub){
  std::string idndata;
  scpi_get_idn(idndata);
  if(idndata.find(idnsub) == std::string::npos){
    return 0;
  }
  else{
    return 1;
  }
}

double WKserial::scpi_obtain_double_value(){
  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss(data);
  double value;
  ss>>value;
  if(data.empty()){
    value= INT_MIN;
  }
  return value;
}

int WKserial::scpi_obtain_int_value(){
  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss(data);
  int value;
  ss>>value;
  if(data.empty()){
    value= INT_MIN;
  }

  return value;
}

unsigned int WKserial::scpi_obtain_uint_value(){
  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss(data);
  unsigned int value;
  ss>>value;
  if(data.empty()){
    value= UINT_MAX;
  }
  return value;
}

void WKserial::scpi_obtain_double_vector(std::vector<double> &dvec){
  dvec.clear();
  std::string data;
  scpi_receive_data(data, 1024);  //TODO: There should be a new func to read as more as possible.  scpi_reciece_data(data) 
  std::size_t lpos = data.find_first_of("+-0123456789");
  std::size_t hpos = data.find_first_not_of("+-0123456789.Ee", lpos);
  for(;lpos != std::string::npos; lpos = data.find_first_of("+-0123456789", hpos)){
    hpos = data.find_first_not_of("+-0123456789.Ee", lpos);
    std::string datasub;
    if(hpos == std::string::npos)
      datasub = data.substr(lpos, hpos-lpos);  // number length
    else
      datasub = data.substr(lpos);
    double value;
    std::stringstream ss(datasub);
    ss>>value;
    dvec.push_back(value);
  }
  return;
}
