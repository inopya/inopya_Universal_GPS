/*
 * Version 1.0
 * Universal GPS NEO6/7/8 reader
 */

#include "Universal_GPS_inopya.h"


Universal_GPS_inopya::Universal_GPS_inopya(SoftwareSerial * ss)
{	
	_updated_ok = false;
	data_time = 0;
	serial = ss;
}  


void Universal_GPS_inopya::begin(uint16_t baud)
{
	serial->begin(baud);
}


void Universal_GPS_inopya::set_mode(uint8_t modo)
{
	_mode = modo;
	
}

uint8_t Universal_GPS_inopya::get_mode()
{
	return _mode;
}


void Universal_GPS_inopya::get(void)
{
    uint32_t inicio = millis();
    _updated_ok = false;
	_updated_GGA = false;
	_updated_RMC = false;
    do{
		while (serial->available() > 0 && (_updated_ok == false)){
			char c = serial->read();  		//leer un caracter del bufer del gps
			_updated_ok = almacenar(c);   	//almacenar el carcter en el buffer
		}
    }while ((millis() - inicio < 1200) && (_updated_ok == false));  //a veces puede tardar hasta un segundo en entregar toda la informaion

	if(_updated_ok==true){
		data_time = millis();
	}
}

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  GESTION DE UN BUFFER PARA ALMACENAR LA INFORMACION DEL GPS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

bool Universal_GPS_inopya::almacenar(char c)
{
	buf[pos] = c;
	pos++;

	if(c == '\n') { //fin de linea
		bool FLAG_resultado = procesar_buf();
		memset(buf, '\0', 120); //reset del buffer
		pos = 0;
		return FLAG_resultado;
	}

	if(pos >= 120){ //desbordamiento del buffer
		memset(buf, '\0', 120); //reset del buffer
		pos = 0;
		return false;
	}
	return false;
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  PROCESADO DE BUFFER GPS PARA EXTRAER DATOS UTILES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

bool Universal_GPS_inopya::procesar_buf()
{
	if(strncmp(buf, "$", 1) != 0) {
		return false;
	}

	/* comprobacion del crc de la sentencia recibida */
	if(!check_CRC()){ // crc no invalido
		return false; //return
	}

  /* si el mensaje cumple su crc, ver que tipo de sentencia es */
  
	if(_mode==0 or _mode==1){
		if(strncmp(buf, "$GNGGA", 6) == 0 or strncmp(buf, "$GPGGA", 6) == 0) {
			//Serial.println(F("he encontrado $GNGGA"));    //DEBUG
			decode_GNGGA(); //fecha, hora, nº satelites, altitud
			_updated_GGA = true;
		}
	}

	if(_mode==0 or _mode==2){
		if(strncmp(buf, "$GNRMC", 6) == 0 or strncmp(buf, "$GPRMC", 6) == 0) {
			//Serial.println(F("he encontrado $GNRMC"));    //DEBUG
			decode_GNRMC();
			_updated_RMC = true;
		}
	}

	bool FLAG_resultado = false;
	switch(_mode){  
        case 0: {  /* procesar GxGGA y GxRMC */
			FLAG_resultado = _updated_GGA && _updated_RMC;   
        };break;
        case 1: {  /* procesar solo GxGGA */
			FLAG_resultado = _updated_GGA;
        };break;
        case 2: {  /* procesar solo GxRMC */
			FLAG_resultado = _updated_RMC;
        };break;
	}

	return FLAG_resultado;//true;
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  CONTROL CRC DEL BUFFER 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

bool Universal_GPS_inopya::check_CRC()
{
  if (buf[strlen(buf)-5] == '*') {
    uint16_t sum = Universal_GPS_inopya::convert2HEX(buf[strlen(buf)-4]) * 16;
    sum += Universal_GPS_inopya::convert2HEX(buf[strlen(buf)-3]);
    for (uint8_t i=1; i < (strlen(buf)-5); i++){
      sum ^= buf[i];
    }
    if (sum != 0){
      return false;
    }
    else{
      return true;
    }
  }
  return false;
}

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  CONVERSION A HEXADECIMAL DEL BUFFER PREVIO CONTROL DE SU CRC
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

uint8_t Universal_GPS_inopya::convert2HEX(char c)
{
    if (c < '0')
        return 0;
    if (c <= '9')
        return c - '0';
    if (c < 'A')
        return 0;
    if (c <= 'F')
        return (c - 'A')+10;
    return 0;
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  BUSCAR INFORMACION EN LA SENTENCIA $GNGGA (compatible con $GPGGA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void Universal_GPS_inopya::decode_GNGGA()
{ 
  int control_datos = -1;
  int j = 0;
  char dato[20];
  for(int i=0;i<sizeof(buf);i++){
    if(control_datos > 8){
      memset(dato, '\0', 20);
      j = 0;
      return;
    }
    if(buf[i]!=','){
      dato[j]=buf[i];
      j++;
    }
    else{
      //Serial.print("dato: ");Serial.println(dato);  //DEBUG  linea muy util mientras se creaba esta rutina
      control_datos++;
      switch(control_datos){
        case 1: {  /* mmmmmmmmmmmmmmm RELOJ GPS  mmmmmmmmmmmmmmm */
          char temp[3]; temp[2]='\0';
          temp[0]= dato[0]; temp[1]= dato[1]; hora = atoi(temp);
          temp[0]= dato[2]; temp[1]= dato[3]; minuto = atoi(temp);
          temp[0]= dato[4]; temp[1]= dato[5]; segundo = atoi(temp); 
                 
        };break;
        case 2: {  /* mmmmmmmmmmmmmmm LATITUD  mmmmmmmmmmmmmmm */
         float temp = atof(dato);
         float grados = int(temp/100.0);
         float minutos = fmod(temp, 100.0); 
         latitud = grados + (minutos/60.0);
        };break;
        case 3: {  /* mmmmmmmmmmmmmmm LATITUD  N/S mmmmmmmmmmmmmmm */
          if(dato[0] == 'S'){
            latitud = -latitud;
          }
        };break;
        case 4: {  /* mmmmmmmmmmmmmmm LONGITUD  mmmmmmmmmmmmmmm */   
         float temp = atof(dato);
         float grados = int(temp/100.0);
         float minutos = fmod(temp, 100.0);
         longitud  = grados + (minutos/60.0);
        };break;
        case 5: {  /* mmmmmmmmmmmmmmm LONGITUD  E/W mmmmmmmmmmmmmmm */
          if(dato[0] == 'W'){
            longitud = -longitud;
          }
        };break;
        case 6: {
          //dato no procesado por no ser de interes para mi
        };break;
        case 7: {  /* mmmmmmmmmmmmmmm Nº SATELITES  mmmmmmmmmmmmmmm */   
          satelites = atoi(dato);
        };break;
        case 8: {
          //dato no procesado por no ser de interes para mi
        };break;
        case 9: {  /* mmmmmmmmmmmmmmm ALTITUD  mmmmmmmmmmmmmmm */   
          altitud = atof(dato);
        };break;
      }
      memset(dato, '\0', 20);
      j = 0;
    }
  }
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//  BUSCAR INFORMACION EN LA SENTENCIA $GNRMC (compatible con $GPRMC)
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void Universal_GPS_inopya::decode_GNRMC()
{ 
  int control_datos = -1;
  int j = 0;
  char dato[20];
  for(int i=0;i<sizeof(buf);i++){
    if(control_datos > 8){
      memset(dato, '\0', 20);
      j = 0;
      return;
    }
    if(buf[i]!=','){
      dato[j]=buf[i];
      j++;
    }
    else{
      //Serial.print("dato: ");Serial.println(dato);  //DEBUG  linea muy util mientras se creaba esta rutina
      control_datos++;
      switch(control_datos){
        case 1: {  /* mmmmmmmmmmmmmmm RELOJ GPS mmmmmmmmmmmmmmm */
          char temp[3]; temp[2]='\0';
          temp[0]= dato[0]; temp[1]= dato[1]; hora = atoi(temp);
          temp[0]= dato[2]; temp[1]= dato[3]; minuto = atoi(temp);
          temp[0]= dato[4]; temp[1]= dato[5]; segundo = atoi(temp);            
        };break;
        case 2: { 
          //dato no procesado por no ser de interes para mi
        };break;        
        case 3: {  /* mmmmmmmmmmmmmmm LATITUD mmmmmmmmmmmmmmm */
         float temp = atof(dato);
         float grados = int(temp/100.0);
         float minutos = fmod(temp, 100.0); 
         latitud = grados + (minutos/60.0);
        };break;
        case 4: {  /* mmmmmmmmmmmmmmm LATITUD  N/S mmmmmmmmmmmmmmm */
          if(dato[0] == 'S'){
            latitud = -latitud;
          }
        };break;
        case 5: {  /* mmmmmmmmmmmmmmm LONGITUD mmmmmmmmmmmmmmm */
         float temp = atof(dato);
         float grados = int(temp/100.0);
         float minutos = fmod(temp, 100.0);
         longitud  = grados + (minutos/60.0);
        };break;
        case 6: {  /* mmmmmmmmmmmmmmm LONGITUD  E/W mmmmmmmmmmmmmmm */
          if(dato[0] == 'W'){
            longitud = -longitud;
          }
        };break;
        case 7: {  /* mmmmmmmmmmmmmmm VELOCIDAD mmmmmmmmmmmmmmm */
          //float nudos = atof(dato);
          velocidad = 1.852 * atof(dato);
        };break;
        case 8: {  /* mmmmmmmmmmmmmmm COURSE mmmmmmmmmmmmmmm */
          course = atof(dato);
        };break;
        case 9: {  /* mmmmmmmmmmmmmmm FECHA mmmmmmmmmmmmmmm */
          char temp[3]; temp[2]='\0';
          temp[0]= dato[0]; temp[1]= dato[1]; dia = atoi(temp);
          temp[0]= dato[2]; temp[1]= dato[3]; mes = atoi(temp);
          temp[0]= dato[4]; temp[1]= dato[5]; year = 2000+atoi(temp);
        };break;
      }
      memset(dato, '\0', 20);
      j = 0;
    }
  }
}
