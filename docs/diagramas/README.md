# Diagramas del sistema — Tracker LoRa-APRS

## Diagrama de bloques — Nivel 1
Vista general del sistema con entradas y salidas principales.

![Nivel 1](Diagrama%20primer%20nivel.svg)

### Entradas:
-	Señal GPS: Señal de radiofrecuencia proveniente de los satélites GNSS, recibida por la antena GPS integrada del T-Beam.
-	Energía: Suministro de potencia proporcionado por la batería 18650 a través del gestor de energía AXP192.
-	Firmware/parámetros: Parámetros de configuración (callsign, frecuencia, intervalo TX) ingresados desde una PC vía USB durante la programación. 
### Salidas:
-	Trama LoRa RF: Paquete APRS transmitido en formato LoRa a 433.775 MHz con la posición, velocidad y rumbo del tracker.
-	Debug serial: Información de estado del sistema enviada por UART0/USB para monitoreo durante desarrollo.
-	Indicadores: LED1 indica estado de carga de batería; LED2 indica actividad del GPS.


## Diagrama de bloques — Nivel 2
Módulos internos del sistema y flujo de datos entre ellos.

![Nivel 2](Diagrama%20segundo%20nivel.svg)

### Módulo de gestión de energía:
Es un módulo pasivo respecto al flujo lógico del sistema. Su única responsabilidad es garantizar alimentación estable. No toma decisiones ni genera señales de control.
### Módulo de adquisición GPS:
Genera tramas NMEA continuamente a 1 Hz. Cada trama indica si el fix es válido (A) o no (V). El módulo de procesamiento es quien interpreta ese campo, no el GPS.
### Módulo de procesamiento:
Es el núcleo lógico del sistema. Ejecuta un bucle de lectura GPS con timeout de 60 segundos. Si obtiene fix antes del timeout, construye la trama y la envía al módulo LoRa. Al recibir TX done, o al cumplirse el timeout sin fix, ordena al control de ciclo que inicie el deep sleep.
### Módulo de transmisión LoRa:
Recibe la trama APRS, la transmite con los parámetros LoRa configurados y notifica al procesamiento cuando la transmisión concluye.
### Módulo de control de ciclo:
Al recibir la orden de sleep, configura el timer Real Time Clock (RTC) interno del ESP32 con el intervalo definido y ejecuta el deep sleep. El RTC es el único componente que permanece activo durante el reposo. Al cumplirse el tiempo, reinicia el ESP32 y el ciclo comienza nuevamente desde el procesamiento.

## Diagrama de bloques - Nivel 3
Expansión del procesamiento en el Firmware.

![Nivel 3](Diagrama%20tercer%20nivel.svg)

En este nivel se hace una ampliación del procesamiento del Firmware, donde se especifican los procesos que realiza. Esta inicia con la obtención de la trama NMEA proveniente del módulo GPS por medio de UART, donde posteriormente se realiza el parseo para obtener la información de interés, como las coordenadas y la señal de si existe "fix" o no. Esta última es la que se analiza para comprobar si se puede transmitir o no los datos obtenidos, donde si se obtiene A se dice que "hay fix", en caso de tener V en ese espacio, se dice que "no hay fix". 

Del mismo Parser se pasa el resto de información necesaria para la creación de las tramas APRS en caso de que se pueda transmitir. Una vez se tiene la trama APRS se transmite por medio de SPI al módulo de LoRa, que es el encargado de realizar la transmisión por medio de la antena a 433.775 MHz.

Todo esto se realiza respetando el control de ciclo especificado, donde más adelante por medio de la máquina de estados se puede apreciar mejor la secuencia del programa.

# Máquina de estados del Firmware

![maquina de estados](maquina_de_estados.svg)

### Estados

| Estado | Descripción |
|---|---|
| **INIT** | Inicialización del sistema: UART, I²C, AXP192, GPS y LoRa. Si falla alguna inicialización, el sistema hace reboot. |
| **ESPERAR_GPS_FIX** | Lectura continua de tramas NMEA a 1 Hz. Se mantiene en este estado mientras las tramas indican sin fix (campo `V`) y el tiempo no supera los 60 segundos. |
| **CONSTRUCCION_TRAMA_APRS** | Convierte los datos GPS al formato APRS y arma el paquete completo con callsign `TI0TEC7-7`, símbolo y extensión de datos. |
| **TRANSMITIR_LoRa** | Envía el paquete APRS por RF a 433.775 MHz mediante el chip SX1276. |
| **DEEP_SLEEP** | El ESP32 apaga CPU, RAM y periféricos. Solo el RTC permanece activo contando 60 segundos. Al cumplirse, reinicia el sistema desde INIT. |

### Transiciones

| Desde | Hacia | Condición |
|---|---|---|
| Punto de entrada | INIT | Encendido o wakeup por RTC |
| INIT | ESPERAR_GPS_FIX | Inicialización exitosa |
| INIT | Punto de entrada | Error de inicialización (reinicio) |
| ESPERAR_GPS_FIX | CONSTRUCCION_TRAMA_APRS | Trama NMEA con fix válido (`A`) antes del timeout |
| ESPERAR_GPS_FIX | ESPERAR_GPS_FIX | Trama NMEA sin fix (`V`) y tiempo < 60s |
| ESPERAR_GPS_FIX | DEEP_SLEEP | Timeout 60s sin fix |
| CONSTRUCCION_TRAMA_APRS | TRANSMITIR_LoRa | Trama APRS construida |
| TRANSMITIR_LoRa | DEEP_SLEEP | TX exitoso |
| TRANSMITIR_LoRa | CONSTRUCCION_TRAMA_APRS | TX fallido (reintento) |
| DEEP_SLEEP | Punto de entrada | Wakeup por RTC (60 segundos) |

### Notas
- El GPS permanece encendido durante el deep sleep ya que el intervalo
  de 60 segundos no justifica el tiempo de reconexión que implicaría apagarlo.
- El punto de entrada (ENCENDIDO / WAKEUP RTC) no es un estado como tal, es el punto de entrada para el funcionamiento del proceso.
- El tiempo de reposo y de encendido es provicional, está sujeto a cambios según lo indiquen análisis posteriores.
