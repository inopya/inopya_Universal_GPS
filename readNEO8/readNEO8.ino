/*
  #       _\|/_   A ver..., ¿que tenemos por aqui?
  #       (O-O)        
  # ---oOO-(_)-OOo---------------------------------
   
   
  ##########################################################
  # ****************************************************** #
  # *            DOMOTICA PARA AFICIONADOS               * #
  # *          Lectura datos de GPS Neo6/7/8             * #
  # *          Autor:  Eulogio López Cayuela             * #
  # *                                                    * #
  # *       Versión v1.0      Fecha: 15/02/2020          * #
  # ****************************************************** #
  ##########################################################
*/

#define __VERSION__ "Universal GPS reader NEO_6/7/8\n"

/*
  
 ===== NOTAS DE LA VERSION =====
 

  1.- Lectura e interpreacion de datos desde modulos GPS NEO 6/7/8
      Se interpretan solo las sentencias:
      $GNGGA,  $GPGGA,  $GNRMC  y  $GPRMC
      (y de ellas algunos datos se desprecian)
      
      Para mas informacion sobre todos los tipos de sentencias 
      y los datos emitidos por los satelites consultar: 
      http://aprs.gids.nl/nmea/#rmc



    ** Compilado con Arduino _IDE 1.8.10 windows **     
     Este Sketch usa 9248 bytes, el (28%) del espacio de FLASH ROM
     Las variables Globales usan 544 bytes, el (26%)  del espacio de RAM
 
      
  CONEXIONES:

 =======================
  ARDUINO     GPS NEO6, NEO7, NEO8  (mi neo 8 necesita alimentacion por usb)
 =======================
   D3   --->   Rx 
   D4   --->   Tx  
   GND  --->   GND
   5v   --->   Vcc

*/ 



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#include <SoftwareSerial.h>
#include <Temporizador_inopya.h>
#include <Universal_GPS_inopya.h>



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DECLARACION DE CONSTANTES  Y  VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define PIN_Rx         4
#define PIN_Tx         3
#define LED_OnBoard   13



//------------------------------------------------------------------------
// Creaccion de objeto puerto serie software
//------------------------------------------------------------------------

// puerto serie para conectar el modulo GPS
SoftwareSerial gpsPort(PIN_Rx, PIN_Tx);

Universal_GPS_inopya NEO_gps(&gpsPort);   //creamos un objeto gps pero diciendole el puerto
                                          //simplifica el acceso a la recogida  de datos


//------------------------------------------------------------------------
// Creaccion de objetos del tipo Temporizador_inopya, 
// uno para cada una de las tareas
//------------------------------------------------------------------------

/* 
 * Los temporizadores virtuales disponen de dos metodos: begin() y estado()
 * 
 *  nombreTemporizador.begin(tiempoEnMilisegundos) --> carga tiempo en el temporizador y lo inicia.
 *  nombreTemporizador.estado() --> retorna un valor booleano: 
 *                                                              true = temporizador aun activo; 
 *                                                              false = temporizador terminado
 */
 
Temporizador_inopya updateGPS;
Temporizador_inopya parpadeoLED;


int contador = 0;


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
//***************************************************************************************************
//         FUNCION DE CONFIGURACION
//***************************************************************************************************
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 

void setup()
{
  Serial.begin(115200);
  NEO_gps.begin(9600);  //iniciamos el GPS a la velocidad standard
  
  pinMode(LED_OnBoard, OUTPUT);
  
  Serial.print(F("GPS por defecto en modo: "));Serial.println(NEO_gps.get_mode());
  
  //NEO_gps.set_mode(1);  //cambiamos a modo 1 decodificar solo GNGGA/GPGGA.
  //uint8_t modo_actual = NEO_gps.get_mode();
  //Serial.print(F("Modo de trabajo actual: "));Serial.print(modo_actual);

  delay(500); //para darle dramatismo a la escena :)
}



//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
//***************************************************************************************************
//  BUCLE PRINCIPAL DEL PROGRAMA   (SISTEMA VEGETATIVO)
//***************************************************************************************************
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 



void loop()
{
  if(parpadeoLED.estado()==false){
    parpadeoLED.begin(250);  //nuestro reloj no obliga, impide que se haga antes de ese tiempo
    digitalWrite(LED_OnBoard, !digitalRead(LED_OnBoard));
  }


  if(updateGPS.estado()==false){
    updateGPS.begin(5000);    //nuestro reloj no obliga, impide que se haga antes de ese tiempo
    NEO_gps.get();            //actualizar la informacion desde el gps
    
    if(contador < 10){ Serial.print(F("0")); }
    if(contador < 100){ Serial.print(F("0")); }
    if(contador < 1000){ Serial.print(F("0")); }
    Serial.print(contador++);
    Serial.print(F(",")); 
    Serial.print(NEO_gps.latitud,6); 
    Serial.print(F(",")); 
    Serial.print(NEO_gps.longitud,6);
    Serial.print(F(","));

    Serial.print(NEO_gps.altitud,2); 
    Serial.print(F(","));
    Serial.print(NEO_gps.velocidad,2);
    Serial.print(F(","));
    Serial.print(NEO_gps.satelites);
    Serial.print(F(","));
    
    if(NEO_gps.hora < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.hora); Serial.print(F(":"));
    if(NEO_gps.minuto < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.minuto); Serial.print(F(":"));
    if(NEO_gps.segundo < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.segundo); 
    
    Serial.print(F(","));
    if(NEO_gps.dia < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.dia); Serial.print(F("/"));
    if(NEO_gps.mes < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.mes); Serial.print(F("/"));
    if(NEO_gps.year < 10){ Serial.print(F("0")); }
    Serial.print(NEO_gps.year);
    
    Serial.println();
  }
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ###################################################################################################### 
                                  BLOQUE DE FUNCIONES AUXILIARES
   ###################################################################################################### 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************
