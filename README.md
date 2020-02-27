# inopya_Universal_GPS
Libreria básica para GPS NEO6, NEO7 y NEO8
Esta pensada para escuchar  al GPS en pines que no sean puerto serie Hardware, por tanto necesita de la libreria softSerial

Para usarla debemos crear un puerto serie que destinaremos al GPS.

***SoftwareSerial gpsPort(PIN_Rx, PIN_Tx);***

e inicializaremos una instancia del GPS pasandole como argumento el puerto serie software

***Universal_GPS_inopya NEO_gps(&gpsPort);*** 

De esta forma se simplifica el acceso a la recogida  de datos evitandonos el control de escucha del puerto
Por defecto la busqueda de datos del GPS esta programada para emplear hasta 1200ms, 
una 'gran perdida' de tiempo a priori, pero con sentencias ***CMR*** a veces necesita incluso mas de un segundo.
'Gastar' un segundo para no obtener datos validos si que es perder el tiempo. 
Puesto que se disponde de un mecanismo de control  (transparente al usuario) que interrumpe la escucha del GPS una vez que encuentra datos validos del tipo pedido, aplicar un margen grande en la busqueda de datos no supone mas que no desaprovehar intentos de muestreo.

Podemos decidir que tipo de sentencias decodificamos.

##Disponible el método **set_mode(uint8_t modo)**  
que puede tomar 3 valores:
- 0, decodifica todo, *GNGGA/GPGGA y GNCMR/GPCMR*.
- 1, decodifica solo *GNGGA/GPGGA*.
- 2, decodifica solo *GNCMR/GPCMR*.

##Disponible el método **get_mode()** 
que nos informa del modo en que estamos trabajando (0,1,2)

datos disponibles:

**float 	  latitud;  
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
uint32_t  data_time;** 
  
Segun el ejemplo anterior, una vez creado el objeto **NEO_gps**, podemos acceder a la posicion:

***float latitud_actual = NEO_gps.latitud;***

***float longitud_actual = NEO_gps.longitud;***

