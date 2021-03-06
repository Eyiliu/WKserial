#ifndef WKSERIAL_HH
#define WKSERIAL_HH

#include <string>
#include <vector>

#include <chrono>
#include <ctime>

class WKserial{

public:
  WKserial(std::string name);
  ~WKserial();
  int connect();
  void disconnect();
  int get_fd();
  int get_speed();
  
  void set_write_interval_msec(int msec); //milli sec
  void set_read_timeout_msec(int msec); //milli sec  
  void set_endline(std::string el);
  void set_speed(int speed);
  void set_hardware_flow_control(int ison);
  void set_software_flow_control(int ison);
  void set_raw_output(int israw);
  void set_raw_intput(int israw);
  void set_parity(int databits,int stopbits,int parity);
  void write_data(std::string data);
  void write_line(std::string line);
  void read_data(std::string &str, int length);
  

  void scpi_send_command(std::string cmd);
  void scpi_send_data(std::string data);
  void scpi_receive_data(std::string &data, int maxlen);
  void scpi_print_idn();
  void scpi_get_idn(std::string &idndata);
  int scpi_is_device(std::string idnsub);
  
  double scpi_obtain_double_value();
  int scpi_obtain_int_value();
  unsigned int scpi_obtain_uint_value();
  void scpi_obtain_double_vector(std::vector<double> &dvec);

private:
  int m_fd;
  std::string m_devname;
  std::string m_endline;
  std::chrono::milliseconds m_du_interval_write; 
  std::chrono::milliseconds m_du_timeout_read;
  std::chrono::steady_clock::time_point m_tp_connect;
  std::chrono::steady_clock::time_point m_tp_last_write;
};

#endif
