/*
 * Version 1.0
 */
 
#ifndef Universal_GPS_inopya_h
#define Universal_GPS_inopya_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SoftwareSerial.h>

class Universal_GPS_inopya
{
  /*  ------ public ------  */
	
  public:  
	float 	  latitud;  
	float 	  longitud; 
	float 	  altitud;  
	float 	  velocidad;
	float 	  course;  
	uint8_t   satelites;
	uint8_t   hora; 
	uint8_t   minuto;   
	uint8_t   segundo;  
	uint8_t   dia;  
	uint8_t   mes;  
	uint16_t  year;
	uint32_t  data_time;
	

    Universal_GPS_inopya(SoftwareSerial * ss);	
	
	void begin(uint16_t baud);
	void get(void);	

	void set_mode(uint8_t modo);
	uint8_t get_mode();
	void timeout(uint16_t timeout);

  /*  ------ private ------  */              
  private:

	char buf[120];   // buffer para el almacenamiento de caracteres RAW del GPS
	uint8_t pos=0;   // 'puntero' del buffer de caracteres
	volatile bool _updated_ok = false;
	volatile bool _updated_GGA = false;
	volatile bool _updated_RMC = false;
	uint16_t _timeout = 1200;
	
	uint8_t _mode = 0;  		// por defecto procesa GxGGA y GxRMC,
								// hora, latitud, longitud, altitud, nº satelites, velocidad, course y fecha
	
 
	SoftwareSerial * serial;
	
	bool almacenar(char c);
	bool procesar_buf();
	bool check_CRC();
	uint8_t convert2HEX(char c);
	void decode_GNGGA();
	void decode_GNRMC();
  
};


#endif 

